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


class StylingWidget(Basf2Widget):
    """The css string for styling the notebook."""
    css_string = """
        #notebook {
            background-color: rgba(20, 166, 255, 0.3);
            background-image: url("http://www-ekp.physik.uni-karlsruhe.de/~nbraun/belle.svg");
            background-repeat: no-repeat;
            background-position: right bottom;
        }

        #notebook-container {
            width: 90%;
        }

        #menubar-container {
            width: 90%;
        }

        #header-container {
            width: 90%;
        }

        .stat-table tr:nth-child(even) {
            background: #FFF
        }
        .stat-table tr:nth-child(2n+3) {
            background: #EEE
        }
        .stat-table tr:first-child {
            background: #AAA;
            font-weight: bold
        }
        .stat-table tr:last-child {
            border: 1px solid black;
            font-weight: bold
        }
        """

    """Create the styling widget."""

    def create(self):
        from IPython.core.display import HTML, display
        html = HTML("<style>\n%s\n</style>" % self.css_string)
        return html


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

    def __init__(self, path, standalone=False):
        """
        Create a new path viewer object with the path from the process
        """
        #: The path to show
        try:
            self.path = path.modules()
        except:
            self.path = path

        self.standalone = standalone

    def create(self):
        """
        Create the widget
        """

        import ipywidgets as widgets

        if self.path is None:
            return widgets.HTML("No modules in path.")

        a = widgets.Accordion()
        children = []

        for i, element in enumerate(self.path):
            html = ModuleViewer(element, standalone=self.standalone)
            children.append(html.create())
            if isinstance(html.module.name, str):
                a.set_title(i, html.module.name)
            else:
                a.set_title(i, html.module.name())

        a.children = children

        return a


class ModuleViewer(Basf2Widget):
    """
    A widget for showing module parameter with their content (not standalone)
    or with their description (standalone).
    """

    def __init__(self, module, standalone=True):
        """ Init with a module as a string or a registered one. """
        if isinstance(module, str):
            import basf2
            self.module = basf2.register_module(module)
        else:
            self.module = module
        self.standalone = standalone

        #: Template for the table beginning
        self.table_beginning_html = """<table style="margin-left: auto; margin-right: auto;
                                       border-collapse: separate; border-spacing: 0px;">"""

        #: Template for the table cell
        self.td_html = "style=\"padding: 10px;\""

        #: Template for the row of parameters
        self.table_row_parameters = """<tr><td {td_style}>{param.name}</td>
                                      <td{color_text} {td_style}>{param.values}</td>
                                      <td style="color: gray; {td_style}>{param.default}</td></tr>"""

        #: Template for the row with help
        self.table_row_help = """<tr><td {td_style}>{param.name}</td>
                                      <td {td_style}>{param.type}</td>
                                      <td {td_style}>{param.values}</td>
                                      <td style="color: gray; {td_style}>{param.description}</td></tr>"""

        #: Template for the simple row
        self.table_row_html_single = """<tr><td colspan="4" {td_style}>{text}</td></tr>"""

        #: Template for the table title
        self.table_title_html = """<thead><td colspan="4" style="text-align: center;
                                   font-size: 18pt;" {td_style}>{module_name} ({package})</td></thead>"""

    def get_color_code(self, param):
        """
         Handy function for getting a color based on a parameter:
         if it has the default value, no color,
         if not, red color.
        """
        if str(param.values) != str(param.default) and str(param.default) != "":
            color_text = " style='color: red;'"
        else:
            color_text = ""
        return color_text

    def create(self):
        """
        Show the widget.
        """
        import ipywidgets as widgets

        html = widgets.HTML()
        html.value = self.table_beginning_html

        if self.standalone:
            if isinstance(self.module.name, str):
                module_name = self.module.name
            else:
                module_name = self.module.name()

            html.value += self.table_title_html.format(module_name=module_name, package=self.module.package(),
                                                       td_style=self.td_html)

            html.value += self.table_row_html_single.format(text=self.module.description(), td_style=self.td_html)

        if len(self.module.available_params()) == 0:
            html.value += self.table_row_html_single.format(text="No parameters available.", td_style=self.td_html)
        else:
            for param in self.module.available_params():
                color_text = self.get_color_code(param)

                if self.standalone:
                    table_row_html = self.table_row_help
                else:
                    table_row_html = self.table_row_parameters

                html.value += table_row_html.format(param=param, color_text=color_text, td_style=self.td_html)
        html.value += "</table>"

        return html


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

        #: Template for a table row
        self.table_row_html = """<tr><td style="padding: 10px 0;">{name}</td>
                                 <td style="padding: 10px 0;">{number}</td></tr>"""

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
            html.value = """<table style="border-collapse: separate; border-spacing: 50px 0;">"""
            for store_array in event["store_content"]:
                html.value += self.table_row_html.format(name=store_array[0], number=store_array[1])
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

        #: Template for a table cell
        self.table_cell_html = """<td style="padding: 10px;">{content}</td>"""
        self.table_cell_col_3_html = """<td colspan="3" style="padding: 10px;">{content}</td>"""
        self.table_cell_left_html = """<td style="padding: 10px; text-align: left">{content}</td>"""

    def create(self):
        """
        Create the widget
        """
        import ipywidgets as widgets

        if self.statistics is None:
            return widgets.HTML("")

        html = widgets.HTML()
        html.value = """<table class="stat-table" style="border-collapse: collapsed; border: 1px solid black;"><tr>"""
        html.value += self.table_cell_html.format(content="Name")
        html.value += self.table_cell_html.format(content="Calls")
        html.value += self.table_cell_html.format(content="Memory(MB)")
        html.value += self.table_cell_html.format(content="Time(s)")
        html.value += self.table_cell_col_3_html.format(content="Time(ms)/call")
        html.value += "</tr>"

        for module in self.statistics.module:
            html.value += "<tr>"
            html.value += self.table_cell_left_html.format(content=module.name)
            html.value += self.table_cell_left_html.format(content=module.calls["EVENT"])
            html.value += self.table_cell_left_html.format(content=(module.memory_sum["EVENT"] / 1024))
            html.value += self.table_cell_left_html.format(content=(module.time_sum["EVENT"] / 1e9))
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


class LogViewer(Basf2Widget):

    def __init__(self, log_content):
        """ Initialize the log viewer.
        """

        #: The log content to show.
        self.log_content = log_content

        #: The info line
        self.info_line = """<tr style="color: black;" class="log-line-info"><td>{content}</td></tr>"""

        #: The debug line
        self.debug_line = """<tr style="color: gray;" class="log-line-debug"><td>{content}</td></tr>"""

        #: The result line
        self.result_line = """<tr style="color: darkgreen;" class="log-line-result"><td>{content}</td></tr>"""

        #: The warning line
        self.warning_line = """<tr style="color: orange;" class="log-line-warning"><td>{content}</td></tr>"""

        #: The error line
        self.error_line = """<tr style="color: red;" class="log-line-error"><td>{content}</td></tr>"""

        #: The fatal line
        self.fatal_line = """<tr style="color: gray;" class="log-line-fatal"><td>{content}</td></tr>"""

        #: The default line
        self.default_line = """<tr class="log-line-default"><td>{content}</td></tr>"""

        #: The toggle button
        self.toggle_button_line = """<a onclick="$('.log-line-{type_lower}').toggle();"
                                  " style="cursor: pointer; margin: 10px;">Toggle {type_upper}</a>"""

    def create(self):
        """
        Create the log viewer.
        """
        from ipywidgets import HTML, Button
        html = HTML()

        html.value = """<div style="max-height: 400px; overflow-y: auto; padding: 5px;">"""

        for type in ["INFO", "DEBUG", "WARNING", "ERROR", "RESULT", "FATAL", "default"]:
            html.value += self.toggle_button_line.format(type_lower=type.lower(), type_upper=type.upper())

        html.value += """<table style="word-break: break-all; margin: 10px;">"""

        for line in self.log_content.split("\n"):
            if line.startswith("[INFO]"):
                html.value += self.info_line.format(content=line)
            elif line.startswith("[DEBUG]"):
                html.value += self.debug_line.format(content=line)
            elif line.startswith("[RESULT]"):
                html.value += self.result_line.format(content=line)
            elif line.startswith("[WARNING]"):
                html.value += self.warning_line.format(content=line)
            elif line.startswith("[ERROR]"):
                html.value += self.error_line.format(content=line)
            elif line.startswith("[FATAL]"):
                html.value += self.fatal_line.format(content=line)
            else:
                html.value += self.default_line.format(content=line)

        html.value += "</table></div>"

        return html
