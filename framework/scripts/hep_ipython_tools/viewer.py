#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numbers
import random
import string
import time

from dateutil.relativedelta import relativedelta


class IPythonWidget(object):
    """
    A base class for widgets in the hep ipython projects.
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


class StylingWidget(IPythonWidget):
    """The css string for styling the notebook."""

    #: The css string for styling the notebook.
    css_string = """
        #notebook-container {
            width: 90%;
        }

        #menubar-container {
            width: 90%;
        }

        #header-container {
            width: 90%;
        }
        """

    def create(self):
        """Create the styling widget."""
        from IPython.core.display import HTML, display
        html = HTML("<style>\n%s\n</style>" % self.css_string)
        return html


class ProgressBarViewer(IPythonWidget):
    """
    Viewer Object used to print data to the IPython Notebook.
    Do not use it on your own.
    """

    def __init__(self):
        """
        Create a new progress bar viewer.
        """
        from IPython.core.display import display
        from ipywidgets import FloatProgress, VBox, Layout, Label

        #: The starting time of the process
        self.last_time = time.time()
        #: The starting percentage (obviously 0)
        self.last_percentage = 0

        self.progress_bar = FloatProgress(value=0, min=0, max=1,
                                          layout=Layout(width="100%", height="40px"))
        self.progress_label = Label()
        # box widget containing progress bar and status label
        self.progress_box = VBox([self.progress_bar, self.progress_label])

        display(self.progress_box)

    def update(self, text_or_percentage):
        """
        Update the widget with a new event number
        """

        from IPython.core.display import display, Javascript

        if isinstance(text_or_percentage, numbers.Number):
            # text_or_percentage is percentage fraction
            current_percentage = float(text_or_percentage)
            current_time = time.time()

            remaining_percentage = 1.0 - current_percentage

            time_delta = current_time - self.last_time
            percentage_delta = current_percentage - self.last_percentage

            if percentage_delta > 0:
                time_delta_per_percentage = 1.0 * time_delta / percentage_delta

                # creates a human-readable time delta like '3 minutes 34 seconds'
                attrs = ['years', 'months', 'days', 'hours', 'minutes', 'seconds']

                def human_readable(delta): return ['%d %s' % (getattr(delta, attr), getattr(delta, attr) > 1 and attr or attr[:-1])
                                                   for attr in attrs if getattr(delta, attr)]

                times_list = human_readable(relativedelta(seconds=time_delta_per_percentage * remaining_percentage))
                human_readable_str = " ".join(times_list)

                display_text = "%d %% Remaining time: %s" % (
                    100 * current_percentage, human_readable_str)

                self.progress_label.value = display_text
                self.progress_bar.value = float(current_percentage)

            else:
                js = ""

        else:
            # text_or_percentage is status string
            self.progress_label.value = "Status: {}".format(text_or_percentage)
            if "finished" in str(text_or_percentage):
                self.progress_bar.value = 1.0
                self.progress_bar.bar_style = "success"
            elif "failed" in str(text_or_percentage):
                self.progress_bar.bar_style = "danger"
                # color box red to see failure even when progress value is 0
                self.progress_box.box_style = "danger"

    def show(self):
        """
        Display the widget
        """
        from IPython.core.display import display

        display(self.progress_box)


class CollectionsViewer(IPythonWidget):
    """
    Viewer object for the store entries.
    Do not use it on your own.
    """

    def __init__(self, collections):
        """
        Create a new collections viewer with the collections object from the process.
        Collections must be a StoreContentList with a list of StoreContents.
        """
        #: The collections to show
        self.collections = collections

        #: Template for a table row
        self.table_row_html = """<tr><td style="padding: 10px 0;">{content.name}</td>
                                 <td style="padding: 10px 0;">{content.number}</td></tr>"""

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
            for store_content in event.content:
                html.value += self.table_row_html.format(content=store_content)
            html.value += "</table>"
            children.append(html)
            a.set_title(i, "Event " + str(event.event_number))

        a.children = children

        return a


class StatisticsViewer(IPythonWidget):
    """
    A viewer widget for displaying the
    statistics in a nicer way in ipython
    """

    def __init__(self, statistics):
        """ Init the widget with the statistics from the process.
        The statistics must be an instance of Statistics. """
        #: The statistics we want to show
        self.statistics = statistics

        #: Template for a table cell
        self.table_column_html = """<td style="padding: 10px;">{content}</td>"""
        #: Template for a table cell spanning 3 columns
        self.table_column_3_html = """<td colspan="3" style="padding: 10px;">{content}</td>"""
        #: Template for a table cell with left alignment
        self.table_cell_html = """<td style="padding: 10px; text-align: left">{content}</td>"""
        #: Template for a table cell with 3 columns
        self.table_cell_3_html = """<td style=\"text-align: right\">{content[0]}</td><td>{content[1]}</td><td>{content[2]}</td>"""

    def create(self):
        """
        Create the widget
        """
        import ipywidgets as widgets

        if self.statistics is None:
            return widgets.HTML("")

        html = widgets.HTML()
        html.value = """<table style="border-collapse: collapsed; border: 1px solid black;">
                        <thead><tr style="background: #AAA; font-weight: bold">"""

        for column in self.statistics.columns:
            if column.three_column_format:
                html.value += self.table_column_3_html.format(content=column.display_name)
            else:
                html.value += self.table_column_html.format(content=column.display_name)
        html.value += "</tr></thead><tbody>"

        for n, module in enumerate(self.statistics.modules):
            if n % 2 == 1:
                html.value += """<tr style="background: #EEE;">"""
            else:
                html.value += """<tr>"""

            for column in self.statistics.columns:
                if column.three_column_format:
                    html.value += self.table_cell_3_html.format(content=module[column.name])
                else:
                    html.value += self.table_cell_html.format(content=module[column.name])

            html.value += "</tr>"

        # SUMMARY html.value += """<tr style="border: 1px solid black; font-weight: bold">"""

        html.value += "</tbody></table>"
        html.margin = "10px"
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


class LogViewer(IPythonWidget):
    """
    A widget to show the log of a calculation.
    """

    def __init__(self, log_content):
        """
        Initialize the log viewer.
        """

        #: The log content to show.
        self.log_content = log_content

        #: The log levels of the framework
        self.log_levels = ["DEBUG", "ERROR", "FATAL", "INFO", "RESULT", "WARNING", "DEFAULT"]

        #: The color codes for the log messages
        self.log_color_codes = {"DEBUG": "gray", "ERROR": "red", "FATAL": "red", "INFO": "black", "RESULT": "green",
                                "WARNING": "orange", "DEFAULT": "black"}

        #: A templated line in the log
        self.log_line = """<tr style="color: {color};" class="log-line-{type_lower}"><td>{content}</td></tr>"""

        #: The toggle button
        self.toggle_button_line = """<a onclick="$('.log-line-{type_lower}').hide();
                                                 $('.log-line-{type_lower}-hide-button').hide();
                                                 $('.log-line-{type_lower}-show-button').show();"
                                        style="cursor: pointer; margin: 0px 10px;"
                                        class="log-line-{type_lower}-hide-button">Hide {type_upper}</a>
                                     <a onclick="$('.log-line-{type_lower}').show();
                                                 $('.log-line-{type_lower}-hide-button').show();
                                                 $('.log-line-{type_lower}-show-button').hide();"
                                        style="cursor: pointer; margin: 0px 10px; display: none;"
                                        class="log-line-{type_lower}-show-button">Show {type_upper}</a>"""

    def create(self):
        """
        Create the log viewer.
        """
        from ipywidgets import HTML, HBox, VBox
        html = HTML()

        html.value = """<div style="max-height: 400px; overflow-y: auto; width: 100%";>"""

        buttons = []
        for type in self.log_levels:
            buttons.append(HTML(self.toggle_button_line.format(type_lower=type.lower(), type_upper=type.upper())))

        buttons_view = HBox(buttons)
        buttons_view.margin = "10px 0px"

        html.value += """<table style="word-break: break-all; margin: 10px;">"""

        for line in self.log_content.split("\n"):
            found = False
            for type in self.log_levels:
                type_upper = type.upper()
                type_lower = type.lower()
                color = self.log_color_codes[type_upper]
                if line.startswith("[{type_upper}]".format(type_upper=type_upper)):
                    html.value += self.log_line.format(content=line, type_lower=type_lower, color=color)
                    found = True
                    break

            if not found:
                html.value += self.log_line.format(content=line, type_lower="default",
                                                   color=self.log_color_codes["DEFAULT"])

        html.value += "</table></div>"
        html.width = "100%"
        html.margin = "5px"

        result_vbox = VBox((buttons_view, html))

        return result_vbox
