import basf2.core as _basf2
import sys
import os

from hep_ipython_tools.calculation_process import CalculationProcess
from hep_ipython_tools.ipython_handler_basf2.entities import Basf2CalculationQueueStatistics
import json


class Basf2CalculationProcess(CalculationProcess):

    """
    Overload implementation of the HEPProcess with the correct handling of the path calculation.
    """

    def __init__(self, result_queue, log_file_name, parameters, path, random_seed=None, max_event=0):
        """Create a new basf2 calculation process."""
        #: Random seed to set. None will not set it.
        self.random_seed = random_seed
        #: Path to process.
        self.path = path
        #: The maximum number of events to process. Leave 0 to process al events
        self.max_event = max_event

        super(Basf2CalculationProcess, self).__init__(result_queue=result_queue, log_file_name=log_file_name,
                                                      parameters=parameters)

    def prepare(self):
        """
        A function to prepare a path with the modules given in path.
        """
        if self.path:
            # import late due to side effects with importing ROOT
            from hep_ipython_tools.ipython_handler_basf2 import python_modules
            # Add the progress python module
            self.path.add_module(python_modules.ProgressPython(self.progress_queue_remote))
            # Add the print collections python module
            self.path.add_module(python_modules.PrintCollections(self.result_queue))
        else:
            #: Set is_valid to false to not show this process in any listings.
            self.is_valid = False

    def initialize_output(self):
        """
        Make sure all output by python and or C is written to the same output file
        """
        # reset stdout/stderr (notebooks forward them over zmq)
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

        # open filename
        logfile = open(self.log_file_name, "wb", 0)

        # redirect stdout/stderr to logfile
        os.dup2(logfile.fileno(), sys.stdout.fileno())
        os.dup2(logfile.fileno(), sys.stderr.fileno())

        # reset logging to use the new file descriptors
        _basf2.reset_log()
        _basf2.logging.zero_counters()
        _basf2.logging.add_json()

    def start_process(self):
        """
        The function given to the process to start the calculation.
        Do not call by yourself.
        Resets the logging system, logs onto console and a file and sets the queues
        (the result queue and the process queue) correctly.
        """

        if not self.path:
            return

        try:
            if self.random_seed is not None:
                _basf2.set_random_seed(self.random_seed)

            # setup output capture
            self.initialize_output()

            # and start processing
            _basf2.process(self.path, self.max_event)

            self.result_queue.put("ipython.statistics", Basf2CalculationQueueStatistics(_basf2.statistics))

            # import ROOT late due to all the side effects
            from ROOT import Belle2
            store_arrays = list(Belle2.PyStoreArray.list())
            all_arrays = list(dict(Belle2.DataStore.Instance().getStoreEntryMap(Belle2.DataStore.c_Event)).keys())

            nodes = dict(
                name="",
                children=[
                    dict(
                        index=i,
                        name=store_array,
                        key=store_array,
                        relation=[
                            other_store_array for other_store_array in store_arrays if Belle2.DataStore.relationName(
                                store_array,
                                other_store_array) in all_arrays]) for i,
                    store_array in enumerate(store_arrays)])
            nodes_json = json.dumps(nodes)
            self.result_queue.put("ipython.dependencies", nodes_json)

        finally:
            self.result_queue.queue.close()

            self.progress_queue_remote.send("end")
            self.progress_queue_remote.close()
