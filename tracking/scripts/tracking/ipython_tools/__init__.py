import python_modules
import calculation
import queue

# System import
import sys
import time
import os
import tempfile

# basf2 imports
import basf2
import ROOT
from ROOT import Belle2

# multiprocessing imports
from multiprocessing import Process, Pipe

# Nice display features imports
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from IPython.core.display import Image, display
from tracking.ipython_tools.queue import Basf2CalculationQueueStatistics


class IPythonHandler:

    """
    Handler class to start processes in an IPython notebook in a convenient way.
    From this whole framework you should not need to create any instances by yourself but rather use the
    given ipython handler for this.

    Usage
    -----

    Create a handler object in the beginning of your NB and use the two methods `process`
    and `process_parameter_space` to turn a path or a path creator function into a _Basf2Calculation.
    Do not create calculations on you own.

        from tracking.validation.ipython_handler import handler

        path = ...

        calculation = handler.process(path)

    """

    def __init__(self):
        """
        Each created log file gets registered and deleted if there are more than 100 log files present
        """
        self.log_files = []

    def process(self, path, result_queue=None):
        """
        Turn a path into a _Basf2Calculation that you can start, stop or whatever you want.

        Arguments
        ---------
        path: The basf2 path
        result_queue: The _Basf2CalculationQueue you want to use. Without giving this as a parameter
           the function creates one for you. Create one on your own with the function create_queue.
        """

        if result_queue is None:
            result_queue = queue.Basf2CalculationQueue()

        basf2_process = Basf2Process(path=path, result_queue=result_queue, log_file_name=self.next_log_file_name())
        return calculation.Basf2Calculation([basf2_process])

    def process_parameter_space(self, path_creator_function, *list_of_parameters):
        """
        Create a list of calculations by combining all parameters with all parameters you provide and
        feeding the tuple into the path_creator_function.

        Arguments
        ---------
        path_creator_function: A function with as many input parameters as parameters you provide
        list_of_parameters: As many lists as you want. Every list is one parameter. If you do not want a
           specific parameter constellation to occur, you can return None in your parameter_creator for
           this combination.

        Usage
        -----

            def creator_function(par_1, par_2, par_3):
                path = ... par_1 ... par_2 ... par_3
                return path

            calculations = handler.process_parameter_space(creator_function, [1, 2, 3], ["x", "y", "z"], [3, 4, 5])

        TODO
        ----

        Create a pool of workers!
        """

        calculation_list = calculation.Basf2CalculationList(path_creator_function, *list_of_parameters)
        all_paths, all_queues = calculation_list.create_all_paths()
        process_list = [Basf2Process(path=path,
                                     result_queue=q,
                                     log_file_name=self.next_log_file_name())
                        for path, q in zip(all_paths, all_queues)]
        return calculation.Basf2Calculation(process_list)

    def next_log_file_name(self):
        next_log_file = tempfile.mkstemp()
        self.log_files.append(next_log_file)
        while len(self.log_files) > 100:
            first_log_file = self.log_files.pop(0)
            f = first_log_file[0]
            log_file_name = first_log_file[1]

            f.close()
            os.unlink(log_file_name)
        return next_log_file[1]

    def create_queue(self):
        """
        Create a Basf2Calculation queue. You need to do this if you want to pass it to your modules
        and write to it while processing the events.
        """
        return queue.Basf2CalculationQueue()

# Create a single instance
handler = IPythonHandler()


class Basf2Process(Process):

    """
    A wrapper around the multiprocessing.Process class
    which can handle a given path and a queue.
    Do not create instances on your own but rather use the IPythonHandler for this.
    """

    def __init__(self, path, result_queue, log_file_name, **kwargs):
        self.path = path

        if result_queue is None:
            raise ValueError("Invalid result_queue")

        self.result_queue = result_queue
        self.already_run = False
        self.log_file_name = log_file_name

        if self.path:
            self.progress_queue_local, self.progress_queue_remote = Pipe()
            self.path.add_module(python_modules.ProgressPython(self.progress_queue_remote))
            self.path.add_module(python_modules.PrintCollections(result_queue))

            Process.__init__(self, target=self.start_process, kwargs={"file_name": self.log_file_name}, **kwargs)
        else:
            Process.__init__(self, target=lambda: None, **kwargs)

    def start_process(self, file_name):
        """
        The function given to the process to start the calculation.
        Do not call by yourself.
        Resets the logging system, logs onto console and a file and sets the queues
        (the result queue and the process queue) correctly.
        """

        try:
            basf2.reset_log()
            basf2.logging.zero_counters()
            basf2.log_to_file(file_name)
            basf2.process(self.path)
            self.result_queue.put("basf2.statistics", Basf2CalculationQueueStatistics(basf2.statistics))
        except:
            raise
        finally:
            self.progress_queue_remote.send("end")

    def get_log(self):
        """
        Return the log file content.
        Use the methods of the Basf2Calculation for a better handling.
        """
        return open(self.log_file_name).read()

    def get(self, name):
        """
        Return an item from the result queue. Only gives a result if the calculation has finished.
        Use the Basf2Calculation for a better handling.
        """
        if not self.is_alive():
            return self.result_queue.get(name)

    def get_keys(self):
        """
        Return the names of all item from the result queue. Only gives a result if the calculation has finished.
        Use the Basf2Calculation for a better handling.
        """
        if not self.is_alive():
            return self.result_queue.get_keys()


"""
TODO
"""


def tail_file(file_name):
    f = open(file_name, 'r')
    if not f:
        return

    while True:
        line = f.readline()
        if line:
            print line,
            sys.stdout.flush()
        else:
            time.sleep(0.01)


class QueueDrawer(CDCSVGDisplayModule):

    def __init__(self, queue, label, *args, **kwargs):
        self.queue = queue
        self.label = label
        CDCSVGDisplayModule.__init__(self, interactive=False, *args, **kwargs)
        self.use_cpp = True

        self.file_list = []

    def terminate(self):
        CDCSVGDisplayModule.terminate(self)
        self.queue.put(self.label, self.file_list)

    def new_output_filename(self):
        output_file_name = CDCSVGDisplayModule.new_output_filename(self)
        self.file_list.append(output_file_name)
        return output_file_name


def show_image(filename, show=True):
    os.system("convert " + filename + " " + filename[:-3] + str("png"))
    image = Image(filename[:-3] + str("png"))
    if show:
        display(image)
    return image
