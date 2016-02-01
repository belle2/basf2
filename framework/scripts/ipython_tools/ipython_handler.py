import os
import tempfile

from ipython_tools import viewer, queue, calculation, information, process


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

        #: A list of open log files.
        self.log_files = []

        #: A shortcut for returning information on the basf2 environment.
        self.information = information.Basf2Information()

        #: A shortcut for returning module information
        self.modules = information.ModulesInformation()

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

        basf2_process = process.Basf2Process(path=path, result_queue=result_queue,
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

        process_list = [process.Basf2Process(path=path,
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
