from IPython.core.display import Image, display, Javascript
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore", category=FutureWarning)
    from IPython.html import widgets

import random
import string


class PathViewer(object):

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
            table{
              border-collapse: separate;
              border-spacing: 50px 0;
            }

            td {
              padding: 10px 0;
              }
        </style>"""

    def create(self):
        """
        Create the widget
        """
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

        return a

    def show(self):
        """
        Show the widget
        """
        a = self.create()
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
        """
        Update the widget with a new event number
        """
        js = "set_event_number(\"" + str(text) + "\"); "
        return display(Javascript(js))

    def show(self):
        """
        Display the widget
        """
        display(self)


class CollectionsViewer(object):

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
            table{
              border-collapse: separate;
              border-spacing: 50px 0;
            }

            td {
              padding: 10px 0;
              }
        </style>"""

    def create(self):
        """
        Create the widget
        """

        a = widgets.Tab()
        children = []

        for i, event in enumerate(self.collections):
            html = widgets.HTML()
            html.value = self.styling_text + "<table>"
            for store_array in event.store_content:
                html.value += "<tr>" + "<td>" + store_array[0] + "</td>" + "<td>" + str(store_array[1]) + "</td>" + "</tr>"
            html.value += "</table>"
            children.append(html)
            a.set_title(i, "Event " + str(event.number))

        a.children = children

        return a

    def show(self):
        """
        Show the widget
        """

        a = self.create()
        display(a)


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
        a = widgets.Accordion()
        a.children = self.children
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
