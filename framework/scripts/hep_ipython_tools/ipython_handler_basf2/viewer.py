##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from hep_ipython_tools import viewer
import random
import string


class StylingWidget(viewer.StylingWidget):
    """The css string for styling the notebook."""

    #: The css string for styling the notebook.
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
        """


class PathViewer(viewer.IPythonWidget):

    """
    Viewer object for the ipython_handler_basf2 path.
    Do not use it on your own.
    """

    def __init__(self, path, standalone=False):
        """
        Create a new path viewer object with the path from the process
        """
        #: The path to show
        try:
            self.path = path.modules()
        except BaseException:
            self.path = path

        #: In the standalone mode, the basic parameters of the modules are shown
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


class ModuleViewer(viewer.IPythonWidget):
    """
    A widget for showing module parameter with their content (not standalone)
    or with their description (standalone).
    """

    def __init__(self, module, standalone=True):
        """ Init with a module as a string or a registered one. """

        #: The module to show
        if isinstance(module, str):
            import basf2.core as _basf2
            self.module = _basf2.register_module(module)
        else:
            self.module = module

        #: In the standalone mode, the basic parameters of the modules are shown
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


class DependencyViewer(viewer.IPythonWidget):
    """Show the dependencies in a nice and fancy way :-)"""

    def __init__(self, store_arrays_with_dependencies_JSON):
        """Create a new dependency viewer."""
        #: A JSON from the processing of dependencies.
        self.store_arrays_with_dependencies_JSON = store_arrays_with_dependencies_JSON

        #: Part of the name representing the object for javascript
        self.random_name = ''.join(random.choice(string.ascii_letters) for _ in range(10))
        #: The name representing the object for javascript
        self.element_name = "dependencies_" + self.random_name

        #: Tenplate for the include string
        self.d3_include_string = """<script src="https://d3js.org/d3.v3.min.js" charset="utf-8"></script>"""

        #: Template for the element itself
        self.d3_element_string = f"""<div id="{self.element_name}"></div>"""

        #: Template for the style
        self.style_template = """<style>
        /* d3 related settings */
        .node {
          font: 300 14px "Helvetica Neue", Helvetica, Arial, sans-serif;
          fill: #bbb;
          cursor: pointer;
        }

        .node:hover {
          fill: #000;
        }

        .link {
          stroke: rgba(20, 166, 255, 0.3);
          stroke-width: 2px;
          fill: none;
          pointer-events: none;
        }

        .node:hover,
        .node--source,
        .node--target {
          font-weight: 700;
        }

        .node--source {
          fill: #2ca02c;
        }

        .node--target {
          fill: #d62728;
        }

        .link--source,
        .link--target {
          stroke-opacity: 1;
          stroke-width: 4px;
        }

        .link--source {
          stroke: #d62728;
        }

        .link--target {
          stroke: #2ca02c;
        }
        </style>"""

        #: Template for inserting the node JSON
        self.nodes_template = """<script>var test_nodes = JSON.parse('{nodes_json}');</script>""".format(
            nodes_json=self.store_arrays_with_dependencies_JSON)

        #: Template for the full HTML
        self.viewer_template = self.d3_include_string + self.nodes_template + self.d3_element_string + self.style_template + """
            <script>
            var diameter = 960;
            var radius = diameter / 2;
            var innerRadius = radius - 120;

            var cluster = d3.layout.cluster()
                .size([360, innerRadius])
                .sort(null)
                .value(function(d) { return d.size; });

            var bundle = d3.layout.bundle();

            var line = d3.svg.line.radial()
                .interpolate("bundle")
                .tension(.85)
                .radius(function(d) { return d.y; })
                .angle(function(d) { return d.x / 180 * Math.PI; });

            var svg = d3.select("#""" + self.element_name + """").append("svg")
                .attr("width", diameter)
                .attr("height", diameter)
                .append("g")
                    .attr("transform", "translate(" + radius + "," + radius + ")");

            var link = svg.append("g").selectAll(".link");
            var node = svg.append("g").selectAll(".node");

            var nodes = cluster.nodes(test_nodes);
            var links = relations(nodes);

            link = link
                .data(bundle(links))
                .enter()
                    .append("path")
                    .each(function(d) { d.source = d[0], d.target = d[d.length - 1]; })
                    .attr("class", "link")
                    .attr("d", line);

            node = node
                .data(nodes.filter(function(n) { return !n.children; }))
                .enter()
                    .append("text")
                    .attr("class", "node")
                    .attr("dy", ".31em")
                    .attr("transform", function(d) {
                            return "rotate(" + (d.x - 90) +
                                   ")translate(" + (d.y + 8) + ",0)"
                                   + (d.x < 180 ? "" : "rotate(180)");})
                    .style("text-anchor", function(d) { return d.x < 180 ? "start" : "end"; })
                    .text(function(d) { return d.key; })
                    .on("mouseover", mouseovered)
                    .on("mouseout", mouseouted);

            function mouseovered(d) {
                node.each(function(n) { n.target = n.source = false; });

                link.classed("link--target", function(l) { if (l.target === d) return l.source.source = true; })
                    .classed("link--source", function(l) { if (l.source === d) return l.target.target = true; })
                    .filter(function(l) { return l.target === d || l.source === d; })
                    .each(function() { this.parentNode.appendChild(this); });

                node.classed("node--target", function(n) { return n.target; })
                    .classed("node--source", function(n) { return n.source; });
            }

            function mouseouted(d) {
                link.classed("link--target", false)
                    .classed("link--source", false);

                node.classed("node--target", false)
                    .classed("node--source", false);
            }

            d3.select(self.frameElement).style("height", diameter + "px");

            // Return a list of imports for the given array of nodes.
            function relations(nodes) {
                var map = {},
                relation = [];

                // Compute a map from name to node.
                nodes.forEach(function(d) {
                    map[d.name] = d;
                });

                // For each import, construct a link from the source to target node.
                nodes.forEach(function(d) {
                    if (d.relation) d.relation.forEach(function(i) {
                        relation.push({source: map[d.name], target: map[i]});
                    });
                });

                return relation;
            }
            </script>
            """

    def create(self):
        """
        Create the widget.
        """
        import ipywidgets as widgets
        html = widgets.HTML(self.viewer_template)

        return html
