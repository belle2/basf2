from IPython.core.display import display, Javascript
from IPython.core.display import Image as display_Image
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore", category=FutureWarning)
    from IPython.html import widgets

import random
import string
import os


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
        a = self.create()
        display(a)


class ComparisonImageViewer(Basf2Widget):

    def __init__(self, images1, images2):
        self.images1 = images1
        self.images2 = images2

    def create(self):
        a = widgets.Accordion()

        children = []
        for image1, image2 in zip(self.images1, self.images2):
            html = widgets.Box()

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
            box.children = [i1, i2]

            children.append(box)

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
        self.path = path

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
                    html.value += "<tr>" + "<td>" + param.name + "</td>" + "<td>" + str(param.values) + "</td>" \
                        + "<td style='color: gray'>" + str(param.default) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            if isinstance(module.name, string):
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
        self.random_name = ''.join(random.choice(string.lowercase) for _ in range(10))
        self.js_name = "progress_bar_" + self.random_name

        display(self)

    def _repr_html_(self):
        html = """
        <div id="{js_name}"><div class="progressbar"></div><span class="event_number">Event: not started</span></div>
        """.format(js_name=self.js_name)

        js = """
        <script type="text/Javascript">
        function set_event_number(number, js_name) {
            if(isNaN(number)) {
                $("#" + js_name + " > .event_number").html("Status: " + number + "");
            } else {
                $("#" + js_name + " > .event_number").html("Percentage: " + 100 * number + "");
            }

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

        $(function() {
          $("#""" + self.js_name + """ > .progressbar").progressbar({
            value: false
          });
        });

        </script>
        """

        return html + js

    def update(self, text):
        """
        Update the widget with a new event number
        """
        js = "set_event_number(\"" + str(text) + "\", \"" + self.js_name + "\"); "
        return display(Javascript(js))

    def show(self):
        """
        Display the widget
        """
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
        self.collections = collections

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

    def __init__(self, statistics):
        self.statistics = statistics

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
        self.children = children

    def create(self):
        """
        Create the widget
        """
        a = widgets.Tab()
        for i in xrange(len(self.children)):
            a.set_title(i, "Process " + str(i))
        a.children = [children.create() for children in self.children if children is not None]
        return a

    def show(self):
        """
        Show the widget
        """
        if len(self.children) > 0:
            a = self.create()

        else:
            a = widgets.HTML("<strong>Calculation list empty. Nothing to show.</strong>")

        display(a)
