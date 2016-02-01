#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ipython_tools import viewer
from ipython_tools import queue
import inspect
import time


class Basf2Calculation:

    """
    Create a _Basf2Calculation from the given Basf2Process that handles
    the status of the process and the actions like start, stop or wait_for_end
    Do not create instances of this class by yourself but rather use the IPythonHandler for this.
    """

    def __init__(self, process_list):
        """ Init with the list of processes (possibly empty) """
        #: The process list (possibly empty)
        self.process_list = process_list

    def __iter__(self):
        """
        Make the class iterable over all single processes
        """
        for process in self.process_list:
            yield Basf2Calculation([process])

    def __len__(self):
        """ Return the number of handled processes """
        return len(self.process_list)

    def stop(self, index=None):
        """
        Kill the processes. Please keep in mind that killing a process is normaly accompanied with data loss.
        """
        def f(process):
            if self.is_running(process):
                process.terminate()

        self.map_on_processes(f, index)

    def start(self, index=None):
        """
        Start the processes in the background.
        Raises an error of the process has been started already.
        You can not restart a process. If you want to do so, create another Basf2Calculation instance.
        """
        def f(process):
            if not process.already_run:
                process.start()
                process.already_run = True
            else:
                raise AssertionError("Calculation can not be started twice.")

        self.map_on_processes(f, index)

    def wait_for_end(self, display_bar=True):
        """
        Send the calculation into the foreground by halting the notebook as long as the process is running.
        Shows a progress bar with the number of processed events.
        Please keep in mind that you can not execute cells in the notebook when having called wait_for_end
        (but before - although a calculation is running.).
        """

        if display_bar:
            # Initialize all process bars
            process_bars = {process: viewer.ProgressBarViewer()
                            for process in self.process_list if process.path is not None}

        started_processes = [p for p in self.process_list if p.path is not None]
        running_processes = started_processes
        # Update all process bars as long as minimum one process is running
        while len(running_processes) > 0:

            running_processes = [p for p in started_processes if self.is_running(p)]
            ended_processes = [p for p in started_processes if not self.is_running(p)]

            for process in ended_processes:
                if display_bar:
                    self.show_end_result(process, process_bars)
                    # Only update the process bar once
                    if process in process_bars:
                        del process_bars[process]

            for process in running_processes:
                # Check if the process does not have a path
                if process.path is None:
                    if display_bar:
                        self.show_end_result(process, process_bars)

                # Check if the process is still running. If not set the end result correctly.
                elif not self.is_running(process):
                    if display_bar:
                        self.show_end_result(process, process_bars)

                # Check if there are news from the process python module (a new percentage)
                elif process.progress_queue_local.poll():
                    result = process.progress_queue_local.recv()
                    if result == "end":
                        process.join()

                    elif display_bar:
                        process_bar = process_bars[process]
                        process_bar.update(result)

            time.sleep(0.01)

        if display_bar:
            for process in self.process_list:
                self.show_end_result(process, process_bars)

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
        Calculate a function on all processes and colltect the results if index is None.
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

    def get_modules(self, index=None):
        """
        Return the modules in the given path.
        """
        return self.map_on_processes(lambda process: process.path.modules() if process.path is not None else None, index)

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

    def get_statistics(self, index=None):
        """
        Return the statistics of the process if finished
        """
        def f(process):
            if self.is_finished(process) and not self.has_failed(process):
                return self.get("basf2.statistics", process)
            else:
                raise AssertionError("Calculation has not finished.")

        return self.map_on_processes(f, index)

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

    def show_path(self, index=None):
        """
        Show the underlaying basf2 path in an interactive way
        """

        def f(process):
            if process.path is not None:
                return viewer.PathViewer(process.path)
            else:
                return None

        self.create_widgets_for_all_processes(f, index)

    def show_collections(self, index=None):
        """
        Show some snapshots on the collections.
        Remember to add the PrintCollectionsPython Module for that!
        """

        def f(process):
            if "basf2.store_content" in self.get_keys(process):
                return viewer.CollectionsViewer(self.get("basf2.store_content", process))
            else:
                return None

        self.create_widgets_for_all_processes(f, index)

    def show_statistics(self, index=None):
        """
        Show the statistics in a smart manner
        """

        def f(process):
            if "basf2.statistics" in self.get_keys(process):
                return viewer.StatisticsViewer(self.get("basf2.statistics", process))
            else:
                return None

        self.create_widgets_for_all_processes(f, index)


class Basf2CalculationList:

    """
    Creates a whole list of paths by combining every list element in every list in lists with every other list in lists.
    """

    def __init__(self, path_function, lists):
        """ Init with the path_function and the lists of named parameters """

        #: The path function
        self.path_function = path_function

        #: The lists of parameters
        self.lists = lists

    def create_all_paths(self):
        """
        Create all paths.
        """
        import itertools

        parameter_names_in_list = list(self.lists.keys())
        parameter_values_in_list = list(self.lists.values())

        every_parameter_combination = itertools.product(*parameter_values_in_list)

        every_parameter_combination_with_names = [{parameter_name: parameter_value for parameter_name, parameter_value in zip(
            parameter_names_in_list, parameter_values)} for parameter_values in list(every_parameter_combination)]

        all_queues = [queue.Basf2CalculationQueue() for combination in every_parameter_combination_with_names]

        def f(q, param_combination):
            args, vargs, vwargs, defaults = inspect.getargspec(self.path_function)
            if "queue" in args:
                param_combination.update({"queue": q})

            return self.path_function(**param_combination)

        all_paths = [f(q, parameter_combination)
                     for q, parameter_combination in zip(all_queues, list(every_parameter_combination_with_names))]
        return all_paths, all_queues, list(every_parameter_combination_with_names)
