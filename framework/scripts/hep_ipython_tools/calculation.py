#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import time

from hep_ipython_tools import viewer
from hep_ipython_tools.calculation_process import CalculationProcess

import multiprocessing


class Calculation:

    """
    Create a Calculation from the given Process that handles
    the status of the process and the actions like start, stop or wait_for_end
    Do not create instances of this class by yourself but rather use the IPythonHandler for this.
    """

    def __init__(self, process_list=None):
        """ Init with an empty list of processes """
        #: The process list (possibly empty, even later)
        if process_list:
            self.process_list = process_list
        else:
            self.process_list = []

        #: The calculation process type to use.
        self._calculation_process_type = CalculationProcess

    def __iter__(self):
        """
        Make the class iterable over all single processes
        """
        for process in self.process_list:
            yield Calculation([process])

    def __len__(self):
        """ Return the number of handled processes """
        return len(self.process_list)

    def stop(self, index=None):
        """
        Kill the processes. Please keep in mind that killing a process is normally accompanied with data loss.
        """
        def f(process):
            if self.is_running(process):
                process.terminate()

        self.map_on_processes(f, index)

    def start_batched_and_wait_for_end(self, max_processes=None):
        """
        Start part of the processes and wait for all to finish. If max_processes is None,
        only n processes will be started where n is the number of cores on the current machine.
        As soon as one process is finished, a waiting one will be started in order to fill all cores
        of the machine.

        Parameters:
          max_processes: The number of processes which can be run at the same time
        """
        if not max_processes:
            max_processes = multiprocessing.cpu_count()

        self.ensure_running(max_processes)
        self.wait_for_end(max_processes=max_processes)

    def start(self, index=None):
        """
        Start the processes in the background.
        Raises an error of the process has been started already.
        You can not restart a process. If you want to do so, create another Calculation instance.
        """
        def f(process):
            if not process.already_run:
                process.start()
                process.already_run = True
            else:
                raise AssertionError("Calculation can not be started twice.")

        self.map_on_processes(f, index)

    def ensure_running(self, max_processes):
        """
        Ensure that the max_processes number of processes is running and will start
        processes which are waiting and have not been started yet.
        """

        running_processes = [p for p in self.process_list if self.is_running(p)]

        processes_to_start = max(0, max_processes - len(running_processes))

        for process in self.process_list:
            # start processes as long as processes_to_start is larger than zero
            if not process.already_run and processes_to_start > 0:
                process.start()
                process.already_run = True
                processes_to_start = processes_to_start - 1

    def wait_for_end(self, display_bar=True, send_notification=False, max_processes=None):
        """
        Send the calculation into the foreground by halting the notebook as
        long as the process is running.  Shows a progress bar with the number
        of processed events.  Please keep in mind that you can not execute
        cells in the notebook when having called wait_for_end (but before -
        although a calculation is running.)


        Parameters:
          display_bar: If true, the display bar is used to show in the notebook
                that the computation is complete.
          send_notification: If true, the notify2 library will be used to
                notify the user if the computation is complete. This will only
                work if the jupyter notebook is hosted on the local desktop
                machine.
          max_processes: The maximum number of processes which will be run on
                the machine. This has no effect when start() has been called
                before.  This parameter can not be used directly, but
                start_batched_and_wait_for_end() should be used.
        """

        if display_bar:
            # Initialize all process bars
            process_bars = {process: viewer.ProgressBarViewer()
                            for process in self.process_list if process.is_valid}

        started_processes = [p for p in self.process_list if p.is_valid and p.already_run]
        running_processes = started_processes
        # Update all process bars as long as minimum one process is running
        while len(running_processes) > 0:

            if max_processes:
                self.ensure_running(max_processes)

            started_processes = [p for p in self.process_list if p.is_valid and p.already_run]
            running_processes = [p for p in started_processes if self.is_running(p)]
            ended_processes = [p for p in started_processes if not self.is_running(p)]

            for process in ended_processes:
                if display_bar:
                    self.show_end_result(process, process_bars)
                    # Only update the process bar once
                    if process in process_bars:
                        del process_bars[process]

            for process in running_processes:
                # Check if the process is valid
                if not process.is_valid:
                    if display_bar:
                        self.show_end_result(process, process_bars)

                # Check if the process is still running. If not set the end result correctly.
                elif not self.is_running(process):
                    if display_bar:
                        self.show_end_result(process, process_bars)

                # Check if there are news from the process python module (a new percentage)
                elif process.progress_queue_local.poll():
                    result = process.progress_queue_local.recv()
                    if result != "end" and display_bar:
                        process_bar = process_bars[process]
                        process_bar.update(result)

                else:
                    process.result_queue.fill_results()
                    process.join(timeout=0.01)

            time.sleep(0.01)

        if display_bar:
            for process in self.process_list:
                self.show_end_result(process, process_bars)

        # send notification if requested and notify2 library is available
        if send_notification:
            try:
                import notify2

                notify2.init("basf2")
                n = notify2.Notification("basf2",  # head line
                                         "Calculation finished",  # Description text
                                         "notification-message-im"   # Icon name
                                         )
                n.show()
            except ImportError:
                # re-throw with more useful message
                raise ImportError("notify2 library must be installed to show Desktop notifications.")

    def show_end_result(self, process, process_bars):
        """
        Shows the end result (finished or failed) for all processes in the process_bars list
        """
        if process in process_bars:
            process_bar = process_bars[process]
            if self.has_failed(process):
                process_bar.update("failed!")
            else:
                process_bar.update("finished")

    def map_on_processes(self, map_function, index):
        """
        Calculate a function on all processes and collect the results if index is None.
        Else calculate the function only one the given process or the process number.
        """
        if len(self.process_list) == 1:
            return map_function(self.process_list[0])
        else:
            if index is None:
                return list(map(map_function, self.process_list))
            else:
                if isinstance(index, int):
                    return map_function(self.process_list[index])
                else:
                    return map_function(index)

    def is_running(self, index=None):
        """
        Test if the process is still running
        """
        return self.map_on_processes(lambda process: process.is_alive(), index)

    def is_finished(self, index=None):
        """
        Test if the process has finished
        """
        return self.map_on_processes(lambda process: process.already_run and not self.is_running(process), index)

    def has_failed(self, index=None):
        """
        Test if the process has failed.
        """
        def f(process):
            if self.is_finished(process):
                return process.exitcode != 0
            else:
                raise AssertionError("Calculation has not finished.")

        return self.map_on_processes(f, index)

    def get(self, name, index=None):
        """
        Return the saved queue item with the given name
        """
        def f(process, name):
            try:
                return process.get(name)
            except KeyError:
                return None
        return self.map_on_processes(lambda process: f(process, name), index)

    def get_keys(self, index=None):
        """
        Return the names of the items that were saved in the queue
        """
        return self.map_on_processes(lambda process: process.get_keys(), index)

    def get_log(self, index=None):
        """
        Return the log of the process if finished
        """
        def f(process):
            if self.is_running(process) or self.is_finished(process):
                return process.get_log()
            else:
                raise AssertionError("Calculation has not been started.")

        return self.map_on_processes(f, index)

    def get_status(self, index=None):
        """
        Return a string describing the current status if the calculation
        """
        def f(process):
            if not process.already_run:
                return "not started"
            elif self.is_running(process):
                return "running"
            elif self.has_failed(process):
                return "failed"
            elif self.is_finished(process):
                return "finished"

        return self.map_on_processes(f, index)

    def get_parameters(self, index=None):
        """
        Return the parameters used to start this calculation
        """
        return self.map_on_processes(lambda process: process.parameters, index)

    def create_widgets_for_all_processes(self, widget_function, index=None):
        """
        Create a overview widget for all processes or only one for the given process.
        """

        if len(self.process_list) == 1:
            widget = widget_function(self.process_list[0])
        else:
            if index is None:
                widget = viewer.ProcessViewer(list(map(widget_function, self.process_list)))
            else:
                if isinstance(index, int):
                    widget = widget_function(self.process_list[index])
                else:
                    widget = widget_function(index)

        widget.show()

    def show_log(self, index=None):
        """
        Show the log of the underlying process(es).
        """

        def f(process):
            if self.is_running(process) or self.is_finished(process):
                return viewer.LogViewer(process.get_log())
            else:
                raise AssertionError("Calculation has not been started.")

        self.create_widgets_for_all_processes(f, index)

    def show_statistics(self, index=None):
        """
        Show the statistics in a smart manner
        """

        def f(process):
            if "ipython.statistics" in self.get_keys(process):
                return viewer.StatisticsViewer(self.get("ipython.statistics", process))
            else:
                return None

        self.create_widgets_for_all_processes(f, index)

    def show_collections(self, index=None):
        """
        Show some snapshots on the collections.
        Remember to add the PrintCollectionsPython Module for that!
        """

        def f(process):
            if "ipython.store_content" in self.get_keys(process):
                return viewer.CollectionsViewer(self.get("ipython.store_content", process))
            else:
                return None

        self.create_widgets_for_all_processes(f, index)

    def get_statistics(self, index=None):
        """
        Return the statistics of the process if finished
        """
        def f(process):
            if self.is_finished(process) and not self.has_failed(process):
                return self.get("ipython.statistics", process)
            else:
                raise AssertionError("Calculation has not finished.")

        return self.map_on_processes(f, index)

    def append(self, result_queue, log_file_name, parameters, **kwargs):
        """
        Construct a new process with the given parameters and add it to the process_list.
        """
        self.process_list.append(self._calculation_process_type(result_queue=result_queue, log_file_name=log_file_name,
                                                                parameters=parameters, **kwargs))
