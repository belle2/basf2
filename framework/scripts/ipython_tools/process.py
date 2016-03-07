import os
from multiprocessing import Process, Pipe

from ipython_tools import python_modules
from ipython_tools import queue

import basf2


class Basf2Process(Process):

    """
    A wrapper around the multiprocessing.Process class
    which can handle a given path and a queue.
    Do not create instances on your own but rather use the IPythonHandler for this.
    """

    def __init__(self, path, result_queue, log_file_name, random_seed=None, parameters=None, **kwargs):
        """
        Create a new process. Variables are the basf2 path, the queue used for results,
        the file name for the logs and the random seed (which can be left empty).
        """
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
        #: The queue used for progress bar communication (local end)
        self.progress_queue_local = None
        #: The queue used for progress bar communication (remote end)
        self.progress_queue_remote = None

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

            #: Create the queue for the progress python module
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
