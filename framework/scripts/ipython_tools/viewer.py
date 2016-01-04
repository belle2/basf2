#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import string
import os
import time
import numbers


class Basf2Widget(object):

    """
    A base class for widgets in basf2
    """

    def create(self):
        """
        Override this method!
        """
        return None

    def show(self):
        """
        Show the widget
        """
        from IPython.core.display import display

        a = self.create()
        display(a)


class ComparisonImageViewer(Basf2Widget):

    """
    A widget for comparing two images
    """

    def __init__(self, images1, images2):
        """
        Initialize with the image file names
        """
        #: The first image path
        self.images1 = images1
        #: The second image path
        self.images2 = images2

    def create(self):
        """ Create the widget """

        import ipywidgets as widgets
        a = widgets.Accordion()

        children = []
        for image1, image2 in zip(self.images1, self.images2):
            if not os.path.exists(image1) or not os.path.exists(image2):
                continue

            image1_png = image1[:-3] + str("png")
            if not os.path.exists(image1_png):
                os.system("convert " + image1 + " " + image1_png)

                if not os.path.exists(image1_png):
                    continue

            i1 = widgets.Image()
            i1.width = "50%"
            i1.value = open(image1_png).read()

            image2_png = image2[:-3] + str("png")
            if not os.path.exists(image2_png):
                os.system("convert " + image2 + " " + image2_png)

                if not os.path.exists(image2_png):
                    continue
            i2 = widgets.Image()
            i2.width = "50%"
            i2.value = open(image2_png).read()

            box = widgets.Box()
            box.children = [i1, i2, box]

        a.children = children
        return a


class PathViewer(Basf2Widget):

    """
    Viewer object for the basf2 path.
    Do not use it on your own.
    """

    def __init__(self, path):
        """
        Create a new path viewer object with the path from the process
        """
        #: The path to show
        self.path = path

        #: Some styling we need
        self.styling_text = """
        <style>
            .path-table table{
              border-collapse: separate;
              border-spacing: 50px;
            }

            .path-table td {
              padding: 10px;
              }

        </style>"""

    def create(self):
        """
        Create the widget
        """

        import ipywidgets as widgets

        if self.path is None:
            return widgets.HTML("")

        a = widgets.Accordion()
        children = []

        for i, module in enumerate(self.path.modules()):
            html = widgets.HTML()
            html.value = self.styling_text + "<table class=\"path-table\">"
            if len(module.available_params()) == 0:
                html.value += "<tr><td>No parameter available.</td></tr>"
            else:
                for param in module.available_params():
                    if str(param.values) != str(param.default) and str(param.default) != "":
                        color_text = " style='color: red;'"
                    else:
                        color_text = ""
                    html.value += "<tr>" + "<td>" + param.name + "</td>" + "<td" + color_text + ">" + str(param.values) + "</td>" \
                        + "<td style='color: gray'>" + str(param.default) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            if isinstance(module.name, str):
                a.set_title(i, module.name)
            else:
                a.set_title(i, module.name())

        a.children = children

        return a


class ProgressBarViewer(Basf2Widget):

    """
    Viewer Object used to print data to the IPython Notebook.
    Do not use it on your own.
    """

    def __init__(self):
        """
        Create a new progress bar viewer.
        """
        from IPython.core.display import display

        #: Part of the name representating the object for javascript
        self.random_name = ''.join(random.choice(string.ascii_letters) for _ in range(10))
        #: The name representating the object for javascript
        self.js_name = "progress_bar_" + self.random_name

        #: The starting time of the process
        self.last_time = time.time()
        #: The starting percentage (obviously 0)
        self.last_percentage = 0

        display(self)

    def _repr_html_(self):
        """
        The repr-html method is used to show html output.
        """
        html = """
        <div id="{js_name}"><div class="progressbar"></div><span class="event_number">Event: not started</span></div>
        """.format(js_name=self.js_name)

        js = """
        <script type="text/Javascript">
        function set_event_number(number, js_name) {
            var progressbar = $("#" + js_name + " > .progressbar");
            var progressbarValue = progressbar.find( ".ui-progressbar-value" );

            if(number == "finished") {
                progressbar.progressbar({value: 100});
                progressbarValue.css({"background": '#33CC33'});
            } else if (number == "failed!") {
                progressbar.progressbar({value: 100});
                progressbarValue.css({"background": '#CC3300'});
            } else {
                progressbar.progressbar({value: 100*number});
                progressbarValue.css({"background": '#CCCCCC'});
            }
        }

        function set_event_text(text, js_name) {
            $("#" + js_name + " > .event_number").html(text);
        }

        $(function() {
          $("#""" + self.js_name + """ > .progressbar").progressbar({
            value: false
          });
        });

        </script>
        """

        return html + js

    def update(self, text_or_percentage):
        """
        Update the widget with a new event number
        """

        from IPython.core.display import display, Javascript

        if isinstance(text_or_percentage, numbers.Number):

            current_percentage = float(text_or_percentage)
            current_time = time.time()

            remaining_percentage = 1.0 - current_percentage

            time_delta = current_time - self.last_time
            percentage_delta = current_percentage - self.last_percentage

            if percentage_delta > 0:
                time_delta_per_percentage = 1.0 * time_delta / percentage_delta

                display_text = "%d %% Remaining time: %.2f s" % (
                    100 * current_percentage, time_delta_per_percentage * remaining_percentage)

                js = "set_event_text(\"" + display_text + "\", \"" + self.js_name + "\"); "
                js += "set_event_number(\"" + str(current_percentage) + "\", \"" + self.js_name + "\"); "
            else:
                js = ""

        else:
            js = "set_event_number(\"" + str(text_or_percentage) + "\", \"" + self.js_name + "\"); "
            js += "set_event_text(\"Status: " + str(text_or_percentage) + "\", \"" + self.js_name + "\"); "

        return display(Javascript(js))

    def show(self):
        """
        Display the widget
        """
        from IPython.core.display import display

        display(self)


class CollectionsViewer(Basf2Widget):

    """
    Viewer object for the basf2 store entries.
    Do not use it on your own.
    """

    def __init__(self, collections):
        """
        Create a new collections viewer with the collections object from the process
        """
        #: The collections to show
        self.collections = collections

        #: Some styling we need
        self.styling_text = """
        <style>
            .coll-table {
              border-collapse: separate;
              border-spacing: 50px 0;
            }

            .coll-table td {
              padding: 10px 0;
              }
        </style>"""

    def create(self):
        """
        Create the widget
        """

        import ipywidgets as widgets

        if self.collections is None:
            return widgets.HTML("")

        a = widgets.Tab()
        children = []

        for i, event in enumerate(self.collections):
            html = widgets.HTML()
            html.value = self.styling_text + "<table class=\"coll-table\">"
            for store_array in event["store_content"]:
                html.value += "<tr>" + "<td>" + store_array[0] + "</td>" + "<td>" + str(store_array[1]) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            a.set_title(i, "Event " + str(event["number"]))

        a.children = children

        return a


class StatisticsViewer(Basf2Widget):

    """
    A viewer widget for displaying the
    basf2 statistics in a nicer way in ipython
    """

    def __init__(self, statistics):
        """ Init the widget with the statistics from the basf2 process """
        #: The statistics we want to show
        self.statistics = statistics

        #: Some styling we need
        self.styling_text = """
        <style>
            .stat-table {
              border-collapse: collapsed;
              border: 1px solid black;
            }
            .stat-table td {
                padding: 5px;
            }
            .stat-table tr:nth-child(even) {background: #FFF}
            .stat-table tr:nth-child(2n+3) {background: #EEE}
            .stat-table tr:first-child {
                background: #AAA;
                font-weight: bold
            }
            .stat-table tr:last-child {
                border: 1px solid black;
                font-weight: bold
            }
        </style>"""

    def create(self):
        """
        Create the widget
        """
        import ipywidgets as widgets

        if self.statistics is None:
            return widgets.HTML("")

        html = widgets.HTML()
        html.value = self.styling_text + "<table class=\"stat-table\"><tr>"
        html.value += "<td>Name</td>"
        html.value += "<td>Calls</td>"
        html.value += "<td>Memory(MB)</td>"
        html.value += "<td>Time(s)</td>"
        html.value += "<td colspan=\"3\">Time(ms)/call</td>"
        html.value += "</tr>"

        for module in self.statistics.module:
            html.value += "<tr>"
            html.value += "<td>%s</td>" % module.name
            html.value += "<td style=\"text-align: right\">%d</td>" % module.calls["EVENT"]
            html.value += "<td style=\"text-align: right\">%d</td>" % (module.memory_sum["EVENT"] / 1024)
            html.value += "<td style=\"text-align: right\">%.2f</td>" % (module.time_sum["EVENT"] / 1e9)
            html.value += "<td style=\"text-align: right\">%.2f</td><td>&plusmn;</td><td>%.2f</td>" % (
                module.time_mean["EVENT"] / 1e6, module.time_stddev["EVENT"] / 1e6)
            html.value += "</tr>"

        html.value += "</table>"
        return html


class ProcessViewer(object):

    """
    A widget to summarize all the infromation from different processes.
    Must be filled with the widgets of the single processes
    """

    def __init__(self, children):
        """
        Create a process viewer
        """

        #: The children for each process
        self.children = children

    def create(self):
        """
        Create the widget
        """
        import ipywidgets as widgets

        a = widgets.Tab()
        for i in range(len(self.children)):
            a.set_title(i, "Process " + str(i))
        a.children = [children.create() for children in self.children if children is not None]
        return a

    def show(self):
        """
        Show the widget
        """

        import ipywidgets as widgets
        from IPython.core.display import display

        if len(self.children) > 0:
            a = self.create()

        else:
            a = widgets.HTML("<strong>Calculation list empty. Nothing to show.</strong>")

        display(a)
