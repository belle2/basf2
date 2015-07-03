import matplotlib.pyplot as plt
import seaborn as sb
sb.set()
import pandas as pd
from root_pandas import read_root
import numpy as np


from contextlib import contextmanager
import tempfile
import os
import time
import sys

import basf2
from multiprocessing import Process, Queue, queues, Pipe
from Queue import Empty
import pybasf2

import string
import random

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from IPython.core.display import Image, display, Javascript
try:
    from IPython.html import widgets
except:
    pass
import math
import ROOT
from ROOT import Belle2


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
        pass

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
            result_queue = _Basf2CalculationQueue()

        basf2_process = _Basf2Process(path=path, result_queue=result_queue)
        return _Basf2Calculation(basf2_process)

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

        Create a pool of workers! Return a CalculationList with some small helper functions!
        """

        calculation_list = _Basf2CalculationList(path_creator_function, *list_of_parameters)
        return [self.process(path) for path in calculation_list._create_all_paths() if path]

    def create_queue(self):
        """
        Create a Basf2Calculation queue. You need to do this if you want to pass it to your modules
        and write to it while processing the events.
        """
        return _Basf2CalculationQueue()

# Create a single instance
handler = IPythonHandler()


class _Basf2CalculationQueue():

    """
    This class is a wrapper around a multiprocessing.Queue

    It can be used to send and receive values from the modules while processing the path.
    You can use it to save - for example - filepaths of outputfiles that you create on the fly.
    The added items are all of the type _Basf2CalculationQueueItem.
    The _Basf2CalculationQueue can be used as a dict. After the termination of the underlaying process
    you can access the different entries by their names you gave them when putting them on the queue.
    """

    def __init__(self):
        self.queue = Queue()
        self.results = dict()

    def put(self, name, item, **kwargs):
        """
        Put an item on the queue with the given name. Please keep that adding two items with the same name
        overrides one of them!
        """
        self.queue.put(_Basf2CalculationQueueItem(name, item), **kwargs)

    def _fill_results(self):
        """
        Fill the internal dict with the information of the queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        Do only call this when the underlying process has ended.
        """
        while True:
            try:
                result = self.queue.get_nowait()
                self.results.update({result.name: result.item})
            except Empty:
                return

    def _get(self, name):
        """
        Return the item with the given name or an Exception when it is not found.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self._fill_results()
        return self.results[name]

    def _get_keys(self):
        """
        Return all possible names of items saved in this queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self._fill_results()
        return self.results.keys()


class _Basf2Calculation():

    """
    Create a _Basf2Calculation from the given _Basf2Process that handles
    the status of the process and the actions like start, stop or wait_for_end
    Do not create instances of this class by yourself but rather use the IPythonHandler for this.
    """

    def __init__(self, process):
        if not process:
            raise ValueError("Given process is invalid")

        self.process = process
        self.already_run = False

    def is_running(self):
        """
        Test if the process is still running
        """
        return self.process.is_alive()

    def stop(self):
        """
        Kill the process. Please keep in mind that killing a process is normaly accompanied with data loss.
        """
        if self.is_running():
            self.process.terminate()
        else:
            raise AssertionError("Calculation has not been started or has finished.")

    def start(self):
        """
        Start the process in the background.
        Raises an error of the process has been started already.
        You can not restart a process. If you want to do so, create another _Basf2Calculation instance.
        """
        if not self.already_run:
            self.process.start()
            self.already_run = True
        else:
            raise AssertionError("Calculation can not be started twice.")

    def wait_for_end(self):
        """
        Send the calculation into the foreground by halting the notebook as long as the process is running.
        Shows a progress bar with the number of processed events.
        Please keep in mind that you can not execute cells in the notebook when having called wait_for_end
        (but before - although a calculation is running.).
        """
        if self.already_run:
            f = ProgressBarViewer()
            while (self.is_running()):
                if self.process.progress_queue_local.poll():
                    result = self.process.progress_queue_local.recv()
                    if result == "end":
                        break
                    f.update(result)

            self.process.join()
            if self.has_failed():
                f.update("failed!")
            else:
                f.update("finished")

            self.process.join()
        else:
            raise AssertionError("Calculation has not been started.")

    def is_finished(self):
        """
        Test if the process has finished
        """
        return self.already_run and not self.is_running()

    def has_failed(self):
        """
        Test if the process has failed.
        """
        if self.is_finished():
            return self.process.exitcode != 0
        else:
            raise AssertionError("Calculation has not finished.")

    def get_modules(self):
        """
        Return the modules in the given path.
        """
        return self.process.path.modules()

    def get(self, name):
        """
        Return the saved queue item with the given name
        """
        return self.process._get(name)

    def get_keys(self):
        """
        Return the names of the items that were saved in the queue
        """
        return self.process._get_keys()

    def get_statistics(self):
        """
        Return the statistics of the process if finished
        """
        if self.is_finished() and not self.has_failed():
            return self.get("basf2.statistics")

    def get_log(self):
        """
        Return the log of the process if finished
        """
        return self.process._get_log()

    def get_status(self):
        """
        Return a string describing the current status if the calculation
        """
        if not self.already_run:
            return "not started"
        elif self.is_running():
            return "running"
        elif self.has_failed():
            return "failed"
        elif self.is_finished():
            return "finished"

    def show_path(self):
        """
        Show the underlaying basf2 path in an interactive way
        """

        path_viewer = PathViewer(self.process.path)
        path_viewer.show()

    def show_collections(self):
        """
        Show some snapshots on the collections.
        Remember to add the PrintCollectionsPython Module for that!
        """

        collections_viewer = CollectionsViewer(self.get("basf2.store_content"))
        collections_viewer.show()


class _Basf2CalculationQueueItem():

    """
    A placeholder for a tuple string, object.
    Do not create them by yourself.
    """

    def __init__(self, name, item):
        self.name = name
        self.item = item


class _Basf2Process(Process):

    """
    A wrapper around the multiprocessing.Process class
    which can handle a given path and a queue.
    Do not create instances on your own but rather use the IPythonHandler for this.
    """

    def __init__(self, path, result_queue, **kwargs):
        if path is None:
            raise ValueError("Invalid path")

        self.path = path

        if result_queue is None:
            raise ValueError("Invalid result_queue")

        self.progress_queue_local, self.progress_queue_remote = Pipe()
        self.path.add_module(ProgressPython(self.progress_queue_remote))
        self.result_queue = result_queue

        self._log_file_name = tempfile.mktemp()
        Process.__init__(self, target=self._start_process, kwargs={"file_name": self._log_file_name}, **kwargs)

    def _start_process(self, file_name):
        """
        The function given ti the process to start the calculation.
        Do not call by yourself.
        Resets the logging system, logs onto console and a file and sets the queues
        (the result queue and the process queue) correctly.
        """

        try:
            basf2.reset_log()
            basf2.logging.zero_counters()
            basf2.log_to_file(file_name)
            basf2.process(self.path)
            self.result_queue.put("basf2.statistics", basf2.statistics())
        except:
            raise
        finally:
            self.progress_queue_remote.send("end")

    def _get_log(self):
        """
        Return the log file content.
        Use the methods of the Basf2Calculation for a better handling.
        """
        return open(self._log_file_name).read()

    def _get(self, name):
        """
        Return an item from the result queue. Only gives a result if the calculation has finished.
        Use the Basf2Calculation for a better handling.
        """
        if not self.is_alive():
            return self.result_queue._get(name)

    def _get_keys(self):
        """
        Return the names of all item from the result queue. Only gives a result if the calculation has finished.
        Use the Basf2Calculation for a better handling.
        """
        if not self.is_alive():
            return self.result_queue._get_keys()


class _Basf2CalculationList():

    """
    Creates a whole list of paths by combining every list element in every list in lists with every other list in lists.
    """

    def __init__(self, path_function, *lists):
        self.path_function = path_function
        self.lists = lists

    def _create_all_paths(self):
        """
        Create all paths.
        """
        import itertools

        every_parameter_combination = itertools.product(*(self.lists))
        all_paths = [self.path_function(*parameter_combination) for parameter_combination in every_parameter_combination]

        return all_paths

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


class PathViewer(object):

    """
    Viewer object for the basf2 path
    """

    def __init__(self, path):
        self.path = path

        self.styling_text = """
        <style>
            table{
              border-collapse: separate;
              border-spacing: 50px 0;
            }

            td {
              padding: 10px 0;
              }
        </style>"""

    def show(self):
        a = widgets.Accordion()
        children = []

        for i, module in enumerate(self.path.modules()):
            html = widgets.HTML()
            html.value = self.styling_text + "<table>"
            for param in module.available_params():
                html.value += "<tr>" + "<td>" + param.name + "</td>" + "<td>" + str(param.values) + "</td>" \
                    + "<td style='color: gray'>" + str(param.default) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            a.set_title(i, module.name())

        a.children = children

        display(a)


class ProgressBarViewer(object):

    """
    Viewer Object used to print data to the IPython Notebook.
    Do not use it on your own.
    """

    def __init__(self):
        self.random_name = ''.join(random.choice(string.lowercase) for _ in range(10))
        self.js_name = "progress_bar_" + self.random_name

        display(self)

    def _repr_html_(self):
        html = """
        <div id="{js_name}"><div class="progressbar"></div><span class="event_number">Event: not started</span></div>
        """.format(js_name=self.js_name)

        js = """
        <script type="text/Javascript">
        function set_event_number(number) {
            if(isNaN(number)) {
                $("#""" + self.js_name + """ > .event_number").html("Status: " + number + "");
            } else {
                $("#""" + self.js_name + """ > .event_number").html("Percentage: " + 100 * number + "");
            }

            var progressbar = $("#""" + self.js_name + """ > .progressbar");
            var progressbarValue = progressbar.find( ".ui-progressbar-value" );

            if(number == "finished") {
                progressbar.progressbar({value: 100});
                progressbarValue.css({"background": '#33CC33'});
            } else if (number == "failed!") {
                progressbar.progressbar({value: 100});
                progressbarValue.css({"background": '#CC3300'});
            } else {
                progressbar.progressbar({value: 100*number});
                progressbarValue.css({"background": '#000000'});
            }
        }

        $(function() {
          $("#""" + self.js_name + """ > .progressbar").progressbar({
            value: false
          });
        });

        </script>
        """

        return html + js

    def update(self, text):
        js = "set_event_number(\"" + str(text) + "\"); "
        return display(Javascript(js))

    def show(self):
        display(self)


class CollectionsViewer(object):

    """
    Viewer object for the basf2 path
    """

    def __init__(self, collections):
        self.collections = collections

        self.styling_text = """
        <style>
            table{
              border-collapse: separate;
              border-spacing: 50px 0;
            }

            td {
              padding: 10px 0;
              }
        </style>"""

    def show(self):
        a = widgets.Tab()
        children = []

        for i, event in enumerate(self.collections):
            html = widgets.HTML()
            html.value = self.styling_text + "<table>"
            for store_array in event:
                html.value += "<tr>" + "<td>" + store_array[0] + "</td>" + "<td>" + str(store_array[1]) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            a.set_title(i, "Snapshot " + str(i))

        a.children = children

        display(a)


class QueueDrawer(CDCSVGDisplayModule):

    def __init__(self, queue, label, *args, **kwargs):
        self.queue = queue
        self.label = label
        CDCSVGDisplayModule.__init__(self, interactive=False, *args, **kwargs)
        self.use_cpp = True

    def new_output_filename(self):
        output_file_name = CDCSVGDisplayModule.new_output_filename(self)
        self.queue.put(self.label, output_file_name)
        return output_file_name


def show_image(filename, show=True):
    os.system("convert " + filename + " " + filename[:-3] + str("png"))
    image = Image(filename[:-3] + str("png"))
    if show:
        display(image)
    return image


class PrintCollections(basf2.Module):

    def __init__(self, queue):
        self.queue = queue
        basf2.Module.__init__(self)
        self.store_content_list = []
        self.event_number = 0
        self.total_number_of_events = 0

    def initialize(self):
        import ROOT
        self.total_number_of_events = ROOT.Belle2.Environment.Instance().getNumberOfEvents()

    def event(self):

        if self.total_number_of_events == 0:
            return

        current_percentage = 1.0 * self.event_number / self.total_number_of_events

        if 100 * current_percentage % 10 == 0:
            registered_store_arrays = Belle2.PyStoreArray.list()
            registered_store_objects = Belle2.PyStoreObj.list()

            event_store_content_list = []

            for store_array_name in registered_store_arrays:
                store_array = Belle2.PyStoreArray(store_array_name)
                event_store_content_list.append([store_array_name, len(store_array)])

            for store_array_name in registered_store_objects:
                event_store_content_list.append([store_array_name, 0])

            self.store_content_list.append(event_store_content_list)

        self.event_number += 1

    def terminate(self):
        self.queue.put("basf2.store_content", self.store_content_list)


class ProgressPython(basf2.Module):

    """
    A small module that prints every now and then the event number to the given connection.
    It is used for status viewers. Do not call it by yourself.
    """

    def __init__(self, queue):
        """ Init the module """
        basf2.Module.__init__(self)
        self.queue = queue
        self.event_number = 0
        self.total_number_of_events = 0
        self.queue.send("init")

    def initialize(self):
        """ Send start to the connection """
        self.queue.send("start")

        # Receive the total number of events

        import ROOT
        self.total_number_of_events = ROOT.Belle2.Environment.Instance().getNumberOfEvents()

    def event(self):
        """ Send the event number to the connection """
        if self.total_number_of_events == 0:
            return

        current_percentage = 1.0 * self.event_number / self.total_number_of_events

        if 100 * current_percentage % 5 == 0:
            self.queue.send(current_percentage)

        self.event_number += 1

    def terminate(self):
        """ Send stop to the connection """
        self.queue.send(1)
        self.queue.send("end")


class TrackingValidationResult:
    #: This class represents a loaded validation root file.
    #: It has methods for plotting the mostly needed graphs

    def __init__(self, filename, label=None, color_index=0, additional_information=None):
        self.filename = filename
        self.pr_data = read_root(self.filename, tree_key="pr_tree/pr_tree")
        self.pr_data["is_prompt"] = (
            np.sqrt(self.pr_data.x_truth ** 2 + self.pr_data.y_truth ** 2) < 0.5) & (self.pr_data.is_primary == 1)
        self.mc_data = read_root(self.filename, tree_key="mc_tree/mc_tree")
        self.mc_data["is_prompt"] = (
            np.sqrt(self.mc_data.x_truth ** 2 + self.mc_data.y_truth ** 2) < 0.5) & (self.mc_data.is_primary == 1)

        self.mc_prompts = self.mc_data[self.mc_data.is_prompt == 1]
        self.pr_prompts = self.pr_data[self.pr_data.is_prompt == 1]

        colors = [(.64, .69, .83), (1, 199.0 / 256, 128.0 / 256),
                  (255.0 / 256, 128.0 / 256, 128.0 / 256), "red", "green", "blue", "black", "gray", "lightgreen"]

        self.label = label
        self.color = colors[color_index % len(colors)]

        self.finding_efficiency = None
        self.hit_efficiency = None
        self.fake_rate = None
        self.clone_rate = None
        self.get_figure_of_merits()

        self.additional_information = additional_information

    def get_figure_of_merits(self):
        if self.finding_efficiency is None:
            overview = read_root(self.filename, tree_key="ExpertMCSideTrackingValidationModule_overview_figures_of_merit")
            self.finding_efficiency = overview.finding_efficiency[0]
            self.hit_efficiency = overview.hit_efficiency[0]

            overview = read_root(self.filename, tree_key="ExpertPRSideTrackingValidationModule_overview_figures_of_merit")
            self.clone_rate = overview.clone_rate[0]
            self.fake_rate = overview.fake_rate[0]

        return dict(finding_efficiency=self.finding_efficiency,
                    hit_efficiency=self.hit_efficiency,
                    clone_rate=self.clone_rate,
                    fake_rate=self.fake_rate)

    def get_figures_of_merit_latex(self):
        results = self.get_figure_of_merits()

        latex_string = r'\begin{table}' + "\n"
        latex_string += r'  \begin{tabular}{cc} \toprule' + "\n"
        latex_string += r'    & \\ \midrule' + "\n"
        latex_string += r'    Finding Efficiency & ' + "%.2f" % (100 * results["finding_efficiency"]) + r' \% \\' + "\n"
        latex_string += r'    Hit Efficiency & ' + "%.2f" % (100 * results["hit_efficiency"]) + r' \% \\' + "\n"
        latex_string += r'    Fake Rate & ' + "%.2f" % (100 * results["fake_rate"]) + r' \% \\' + "\n"
        latex_string += r'    Clone Rate & ' + "%.2f" % (100 * results["clone_rate"]) + r' \% \\ \bottomrule' + "\n"
        latex_string += r'  \end{tabular}' + "\n"
        latex_string += r'\end{table}'

        return latex_string

    def plot_efficiency_point(self):
        self.plot(100 * self.finding_efficiency, 100 * self.hit_efficiency, loc=3)
        plt.xlabel("finding efficiency")
        plt.ylabel("hit efficiency")

    def grouped_by_pt_data(self, mc_data=None):
        if mc_data is None:
            mc_data = self.mc_data

        pt_values = pd.cut(mc_data.pt_truth, np.arange(0, 2, 0.1))
        grouped = mc_data.groupby(pt_values)

        return grouped

    def plot(self, data_x, data_y, loc=4):
        plt.plot(data_x, data_y, ls="-", marker="o",
                 color=self.color, label=self.label)

        if self.label is not None:
            plt.legend(loc=loc)

    def plot_finding_efficiency(self, data=None):
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().is_matched)
        plt.xlabel("pt")
        plt.ylabel("finding efficiency")

    def plot_hit_efficiency(self, data=None):
        grouped = self.grouped_by_pt_data(data)

        self.plot(grouped.median().pt_truth, grouped.mean().hit_efficiency)
        plt.xlabel("pt")
        plt.ylabel("hit efficiency")

    def print_useful_information(self):
        pr_data = self.pr_data
        mc_data = self.mc_data
        primaries = pr_data[self.pr_data.is_prompt == 1]
        primaries_mc = mc_data[self.mc_data.is_prompt == 1]

        print self.label
        print "Fake", 100 * primaries.is_fake.mean(), 100 * pr_data.is_fake.mean()
        print "Clone", 100 * primaries.is_clone.mean(), 100 * pr_data.is_clone.mean()
        print "Ghost", 100 * primaries.is_ghost.mean(), 100 * pr_data.is_ghost.mean()
        print "Fitted", 100 * primaries.is_fitted.mean(), 100 * pr_data.is_fitted.mean()
        print "Found", 100 * primaries_mc.is_matched.mean(), 100 * mc_data.is_matched.mean()
        print "Found2", 100.0 - 100 * primaries_mc.is_missing.mean(), 100.0 - 100 * mc_data.is_missing.mean()
        print "Merged", 100 * primaries_mc.is_merged.mean(), 100 * mc_data.is_merged.mean()
        print "Hit-Eff", 100 * primaries_mc.hit_efficiency.mean(), 100 * mc_data.hit_efficiency.mean()
        print "Wrong Hits", primaries.number_of_wrong_hits.mean(), pr_data.number_of_wrong_hits.mean()

    def append_to_dataframe(self, df):
        result = {"finding_efficiency": self.finding_efficiency,
                  "hit_efficiency": self.hit_efficiency,
                  "clone_rate": self.clone_rate,
                  "fake_rate": self.fake_rate,
                  "file_name": self.filename}
        if self.additional_information:
            result.update(self.additional_information)
        return df.append(result, ignore_index=True)


class TMVAPlotter():

    def __init__(self):
        pass

    def grouper(self, data, truth_value, non_truth_value, truth_column):
        if data.size == 0:
            return None
        if (data[truth_column] == 1).all():
            return truth_value
        else:
            return non_truth_value

    def hatcher(self, X, truth_column):
        return self.grouper(X, "//", "", truth_column)

    def filler(self, X, truth_column):
        return self.grouper(X, False, True, truth_column)

    def labeler(self, X, truth_column):
        return self.grouper(X, "signal", "background", truth_column)

    def plot_splitted(self, data, column_name, title=None, truth_column="truth"):
        grouped = data.groupby(data[truth_column])
        for name, X in grouped:
            X[column_name].hist(normed=True, histtype="bar", hatch=self.hatcher(X, truth_column),
                                fill=self.filler(X, truth_column), label=self.labeler(X, truth_column))

        plt.title(title)
        plt.legend()

tmvaPlotter = TMVAPlotter()
