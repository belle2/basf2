#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The needed scrips from this package
from . import python_modules
from . import calculation
from . import queue
from . import viewer

# System imports
import os
import tempfile

# basf2 imports
import basf2

# multiprocessing imports
from multiprocessing import Process, Pipe


class Basf2Information:

    """
    Helper class for acessing the information about basf2
    from the environment variables.
    """

    def __init__(self):
        """
        Get the variables from the environment variables.
        """
        #: Externals version
        self.externals_version = os.environ.get("BELLE2_EXTERNALS_VERSION")
        #: Compile options of externals
        self.externals_option = os.environ.get("BELLE2_EXTERNALS_OPTION")
        #: Compile options of Belle2
        self.option = os.environ.get("BELLE2_OPTION")
        #: OS
        self.architecture = os.environ.get("BELLE2_ARCH")
        #: Release version
        self.release = os.environ.get("BELLE2_RELEASE")

    def __str__(self):
        """
        A nice representation.
        """
        result = ""
        result += "externals_version: " + self.externals_version + "\n"
        result += "externals_option: " + self.externals_option + "\n"
        result += "option: " + self.option + "\n"
        result += "architecture: " + self.architecture + "\n"
        result += "release: " + self.release + "\n"
        return result

    def __repr__(self):
        """
        Also for ipython.
        """
        return self.__str__()


class IPythonHandler:

    """
    Handler class to start processes in an IPython notebook in a convenient way.
    From this whole framework you should not need to create any instances by yourself but rather use the
    given ipython handler for this.

    Usage
    -----

    Create a handler object in the beginning of your NB and use the two methods `process`
    and `process_parameter_space` to turn a path or a path creator function into a Basf2Calculation.
    Do not create calculations on you own.

        from tracking.validation.ipython_handler import handler

        path = ...

        calculation = handler.process(path)

    """

    def __init__(self):
        """
        Each created log file gets registered and deleted if there are more than 20 log files present
        or if the get_log function of the process is called (the log is saved elsewhere).
        As the log files are saved to /tmp you have probably not to care about deleting them.
        """
        self.log_files = []
        self.basf2 = Basf2Information()

    @staticmethod
    def style():
        """
        Show a nice styling :-)
        """
        styling_widget = viewer.StylingWidget()
        styling_widget.show()

    def process(self, path, result_queue=None, random_seed=None):
        """
        Turn a path into a Basf2Calculation that you can start, stop or whatever you want.

        Arguments
        ---------
        path: The basf2 path
        result_queue: The Basf2CalculationQueue you want to use. Without giving this as a parameter
           the function creates one for you. Create one on your own with the function create_queue.
        """

        if result_queue is None:
            result_queue = queue.Basf2CalculationQueue()

        basf2_process = Basf2Process(path=path, result_queue=result_queue,
                                     log_file_name=self.next_log_file_name(),
                                     random_seed=random_seed)
        return calculation.Basf2Calculation([basf2_process])

    def process_parameter_space(self, path_creator_function, **kwargs):
        """
        Create a list of calculations by combining all parameters with all parameters you provide and
        feeding the tuple into the path_creator_function.
        If the path_creator_function has a parameter named queue, the function feeds the corresponding
        created queue into the path_creator_function.

        Please note that a list of calculations acts the same as a single calculation you would get from
        the process function. You can handle 10 calculations the same way you would handle a single one.

        Arguments
        ---------
        path_creator_function: A function with as many input parameters as parameters you provide.
           If the function has an additional queue parameter it is fed with the corresponding queue for this path.
        list_of_parameters: As many lists as you want. Every list is one parameter. If you do not want a
           specific parameter constellation to occur, you can return None in your path_creator_function for
           this combination.

        Usage
        -----

            def creator_function(par_1, par_2, par_3, queue):
                path = ... par_1 ... par_2 ... par_3
                queue.put(..., ...)
                return path

            calculations = handler.process_parameter_space(creator_function, [1, 2, 3], ["x", "y", "z"], [3, 4, 5])
        """

        if "random_seeds" in kwargs:
            random_seeds = kwargs["random_seeds"]
            del kwargs["random_seeds"]
        else:
            random_seeds = None

        calculation_list = calculation.Basf2CalculationList(path_creator_function, kwargs)
        all_paths, all_queues, all_parameters = calculation_list.create_all_paths()

        if isinstance(random_seeds, list):
            all_seeds = random_seeds
            while len(all_seeds) < len(all_paths):
                all_seeds += all_seeds

        else:
            all_seeds = [random_seeds] * len(all_paths)

        process_list = [Basf2Process(path=path,
                                     result_queue=q,
                                     random_seed=random_seed,
                                     parameters=parameters,
                                     log_file_name=self.next_log_file_name())
                        for path, q, random_seed, parameters in zip(all_paths, all_queues, all_seeds, all_parameters)]
        return calculation.Basf2Calculation(process_list)

    def next_log_file_name(self):
        """
        Return the name of the next log file.
        If there are more thann 20 log files present,
        start deleting the oldes ones.
        """
        next_log_file = tempfile.mkstemp()
        self.log_files.append(next_log_file)
        while len(self.log_files) > 20:
            first_log_file = self.log_files.pop(0)
            f = first_log_file[0]
            log_file_name = first_log_file[1]

            os.close(f)
            try:
                os.unlink(log_file_name)
            except OSError:
                pass
        return next_log_file[1]

    @staticmethod
    def create_queue():
        """
        Create a Basf2Calculation queue. You need to do this if you want to pass it to your modules
        and write to it while processing the events.
        """
        return queue.Basf2CalculationQueue()

#: Create a single instance
handler = IPythonHandler()


class Basf2Process(Process):

    """
    A wrapper around the multiprocessing.Process class
    which can handle a given path and a queue.
    Do not create instances on your own but rather use the IPythonHandler for this.
    """

    def __init__(self, path, result_queue, log_file_name, random_seed=None, parameters=None, **kwargs):
        if result_queue is None:
            raise ValueError("Invalid result_queue")

        #: Result queue as a reference
        self.result_queue = result_queue
        #: True if already started/run
        self.already_run = False
        #: Name of the log file to use
        self.log_file_name = log_file_name
        #: Random seed to set. None will not set it
        self.random_seed = random_seed
        #: Saved log file content after the run
        self.log_file_content = None
        #: Path to process
        self.path = path
        #: Parameters in process_parameter_space
        self.parameters = parameters

        if path:
            # Append the needed ToFileLogger module
            created_path = basf2.create_path()
            file_logger_module = basf2.register_module("ToFileLogger")
            file_logger_module.param("fileName", log_file_name)
            created_path.add_module(file_logger_module)

            # Copy the modules from the path
            for module in path.modules():
                created_path.add_module(module)

            self.path = created_path

            # Create the queue for the progress python module
            self.progress_queue_local, self.progress_queue_remote = Pipe()

            # Add the progress python module
            self.path.add_module(python_modules.ProgressPython(self.progress_queue_remote))

            # Add the print collections python module
            self.path.add_module(python_modules.PrintCollections(result_queue))

            Process.__init__(self, target=self.start_process,
                             kwargs={"file_name": self.log_file_name, "random_seed": self.random_seed},
                             **kwargs)
        else:
            Process.__init__(self, target=lambda: None, **kwargs)

    def start_process(self, file_name, random_seed=None):
        """
        The function given to the process to start the calculation.
        Do not call by yourself.
        Resets the logging system, logs onto console and a file and sets the queues
        (the result queue and the process queue) correctly.
        """

        try:
            if random_seed is not None:
                basf2.set_random_seed(random_seed)

            basf2.reset_log()
            basf2.logging.zero_counters()
            basf2.log_to_file(file_name)
            basf2.process(self.path)
            self.result_queue.put("basf2.statistics", queue.Basf2CalculationQueueStatistics(basf2.statistics))
        except:
            raise
        finally:
            self.progress_queue_remote.send("end")

    def save_log(self):
        """
        Delete the log file and copy its content to the class.
        """
        if self.log_file_content is None:
            self.log_file_content = open(self.log_file_name).read()
            os.unlink(self.log_file_name)
            self.log_file_name = None

    def get_log(self):
        """
        Return the log file content.
        Use the methods of the Basf2Calculation for a better handling.
        """
        if self.is_alive():
            return open(self.log_file_name).read()
        else:
            self.save_log()
            return self.log_file_content

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
