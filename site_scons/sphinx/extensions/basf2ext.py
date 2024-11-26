##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This module is a Sphinx Extension for the Belle~II Software:

* add a domain "b2" for modules and variables.
  Modules can be documented using the ``.. b2:module::` directive and variables
  using the ``.. b2:variable::` directive. They can be cross referenced with
  :b2:mod: and :b2:var: respectively
* add an index for basf2 modules
* add a directive to automatically document basf2 modules similar to autodoc

    .. b2-modules::
        :package: framework
        :modules: EventInfoSetter, EventInfoPrinter
        :library: libcore.so
        :regex-filter: Event.*

* add directive to automatically document basf2 variables

    .. b2-variables::
        :group: Kinematics
        :variables: x, y, z
        :regex-filter: .*

"""


import os
import re
import textwrap
from docutils import nodes
from sphinx.util.nodes import nested_parse_with_titles
from docutils.parsers.rst import Directive, directives
from docutils.statemachine import StringList
from basf2domain import Basf2Domain
from basf2 import list_available_modules, register_module
from sphinx.domains.std import StandardDomain


def parse_with_titles(state, content):
    """Shortcut function to parse a reStructuredText fragment into docutils nodes"""
    node = nodes.container()
    # copied from autodoc:  necessary so that the child nodes get the right source/line set
    node.document = state.document
    if isinstance(content, str):
        content = content.splitlines()
    if not isinstance(content, StringList):
        content = StringList(content)
    nested_parse_with_titles(state, content, node)
    return node.children


class RenderDocstring(Directive):
    """Directive to Render Docstring as Docutils nodes.
    This is useful as standard reStructuredText does not parse Google
    Docstrings but we support it in docstrings for python functions, modules
    and variables. So to show example docstrings in the documentation we don't
    want to write the example in Google Docstring and keep that synchronous
    with a reStructuredText version
    """
    #: \cond Doxygen_suppress
    has_content = True
    option_spec = {
        "lines": directives.unchanged
    }
    #: \endcond

    def run(self):
        """Just pass on the content to the autodoc-process-docstring event and
        then parse the resulting reStructuredText."""
        env = self.state.document.settings.env
        content = list(self.content)
        try:
            start_index, end_index = (int(e) for e in self.options.get("lines", None).split(","))
            content = content[start_index:end_index]
        except Exception:
            pass

        # remove common whitespace
        content = textwrap.dedent("\n".join(content)).splitlines()

        env.app.emit('autodoc-process-docstring', "docstring", None, None, None, content)
        return parse_with_titles(self.state, content)


#: \cond Doxygen_suppress
class ModuleListDirective(Directive):
    has_content = False
    option_spec = {
        "library": directives.unchanged,
        "modules": directives.unchanged,
        "package": directives.unchanged,
        "no-parameters": directives.flag,
        "noindex": directives.flag,
        "regex-filter": directives.unchanged,
        "io-plots": directives.flag,
    }
#: \endcond

    def show_module(self, module, library):
        description = module.description().splitlines()
        # pretend to be the autodoc extension to let other events process
        # the doc string. Enables Google/Numpy docstrings as well as a bit
        # of doxygen docstring conversion we have
        env = self.state.document.settings.env
        env.app.emit('autodoc-process-docstring', "b2:module", module.name(), module, None, description)
        description += ["", "",
                        f":Package: {module.package()}",
                        f":Library: {os.path.basename(library)}",
                        ]

        if "no-parameters" not in self.options:
            optional_params = []
            required_params = []
            for p in module.available_params():
                dest = required_params if p.forceInSteering else optional_params
                default = "" if p.forceInSteering else f", default={p.default!r}"
                param_desc = p.description.splitlines()
                # run the description through autodoc event to get
                # Google/Numpy/doxygen style as well
                env.app.emit('autodoc-process-docstring', 'b2:module:param', module.name() + '.' + p.name, p, None, param_desc)
                param_desc = textwrap.indent("\n".join(param_desc), 8 * " ").splitlines()
                dest += [f"    * **{p.name}** *({p.type}{default})*"]
                dest += param_desc

            if(required_params):
                description += [":Required Parameters:", "    "] + required_params
            if(optional_params):
                description += [":Parameters:", "    "] + optional_params

        if "io-plots" in self.options:
            image = f"build/ioplots/{module.name()}.png"
            if os.path.exists(image):
                description += [":IO diagram:", "    ", f"    .. image:: /{image}"]

        content = [f".. b2:module:: {module.name()}"] + self.noindex + ["    "]
        content += ["    " + e for e in description]
        return parse_with_titles(self.state, content)

    def run(self):
        all_modules = list_available_modules().items()
        # check if we have a list of modules to show if so filter the list of
        # all modules
        if "modules" in self.options:
            modules = [e.strip() for e in self.options["modules"].split(",")]
            all_modules = [e for e in all_modules if e[0] in modules]

        # check if we have a regex-filter, if so filter list of modules
        if "regex-filter" in self.options:
            re_filter = re.compile(self.options["regex-filter"])
            all_modules = [e for e in all_modules if re_filter.match(e[0]) is not None]

        # aaand also filter by library (in case some one wants to document all
        # modules found in a given library)
        if "library" in self.options:
            lib = self.options["library"].strip()
            all_modules = [e for e in all_modules if os.path.basenam(e[1]) == lib]

        # see if we have to forward noindex
        self.noindex = ["    :noindex:"] if "noindex" in self.options else []

        # list of all docutil nodes we create to be returned
        all_nodes = []

        # now loop over all modules
        for name, library in sorted(all_modules):
            module = register_module(name)
            # filter by package: can only be done after instantiating the module
            if "package" in self.options and module.package() != self.options["package"]:
                continue

            # everyting set, create documentation for our module
            all_nodes += self.show_module(module, library)

        return all_nodes


#: \cond Doxygen_suppress
class VariableListDirective(Directive):
    has_content = False
    option_spec = {
        "group": directives.unchanged,
        "variables": directives.unchanged,
        "regex-filter": directives.unchanged,
        "description-regex-filter": directives.unchanged,
        "noindex": directives.flag,
    }
#: \endcond

    def run(self):
        from ROOT import Belle2
        manager = Belle2.Variable.Manager.Instance()
        self.noindex = ["    :noindex:"] if "noindex" in self.options else []
        all_variables = []
        explicit_list = None
        regex_filter = None
        desc_regex_filter = None
        if "variables" in self.options:
            explicit_list = [e.strip() for e in self.options["variables"].split(",")]
        if "regex-filter" in self.options:
            regex_filter = re.compile(self.options["regex-filter"])
        if "description-regex-filter" in self.options:
            desc_regex_filter = re.compile(self.options["description-regex-filter"])

        for var in manager.getVariables():
            if "group" in self.options and self.options["group"] != var.group:
                continue
            if explicit_list and var.name not in explicit_list:
                continue
            if regex_filter and not regex_filter.match(var.name):
                continue
            if desc_regex_filter and not desc_regex_filter.match(var.description):
                continue
            all_variables.append(var)

        all_nodes = []
        env = self.state.document.settings.env
        for var in sorted(all_variables, key=lambda x: x.name):

            # for overloaded variables, we might have to flag noindex in the
            # variable description so also check for that
            index = self.noindex
            if ":noindex:" in var.description:
                index = ["    :noindex:"]
                var.description = var.description.replace(":noindex:", "")

            docstring = var.description.splitlines()
            # pretend to be the autodoc extension to let other events process
            # the doc string. Enables Google/Numpy docstrings as well as a bit
            # of doxygen docstring conversion we have
            env.app.emit('autodoc-process-docstring', "b2:variable", var.name, var, None, docstring)

            description = [f".. b2:variable:: {var.name}"] + index + [""]
            description += ["    " + e for e in docstring]
            if "group" not in self.options:
                description += ["", f"    :Group: {var.group}"]

            all_nodes += parse_with_titles(self.state, description)

        return all_nodes


def html_page_context(app, pagename, templatename, context, doctree):
    """Provide Link to GitLab Repository, see https://mg.pov.lt/blog/sphinx-edit-on-github.html

    this goes in conjunction with
    site_scons/sphinx/_sphinxtemplates/sourcelink.html and adds a link to our
    git repository instead to the local source link
    """

    if templatename != 'page.html' or not doctree:
        return

    path = os.path.relpath(doctree.get('source'), app.builder.srcdir)
    repository = app.config.basf2_repository
    if not repository:
        return

    commit = app.config.basf2_commitid
    context["source_url"] = f"{repository}/-/blob/main/{path}"
    if commit:
        context["source_url"] = f"{repository}/-/blob/{commit}/{path}"


def gitlab_issue_role(role, rawtext, text, lineno, inliner, options=None, content=None):
    if content is None:
        content = []
    if options is None:
        options = {}
    issue_url = inliner.document.settings.env.app.config.basf2_issues
    if not issue_url:
        return [nodes.literal(rawtext, text=text, language=None)], []

    url = f"{issue_url}/{text}"
    return [nodes.reference(rawtext, text=text, refuri=url)], []


def setup(app):
    import basf2
    basf2.logging.log_level = basf2.LogLevel.WARNING

    app.add_config_value("basf2_repository", "", True)
    app.add_config_value("basf2_commitid", "", True)
    app.add_config_value("basf2_issues", "", True)
    app.add_domain(Basf2Domain)
    app.add_directive("b2-modules", ModuleListDirective)
    app.add_directive("b2-variables", VariableListDirective)
    app.add_directive("docstring", RenderDocstring)
    app.add_role("issue", gitlab_issue_role)
    app.connect('html-page-context', html_page_context)

    # Sadly sphinx does not seem to add labels to custom indices ... :/
    StandardDomain.initial_data["labels"]["b2-modindex"] = ("b2-modindex", "", "basf2 Module Index")
    StandardDomain.initial_data["labels"]["b2-varindex"] = ("b2-varindex", "", "basf2 Variable Index")
    StandardDomain.initial_data["anonlabels"]["b2-modindex"] = ("b2-modindex", "")
    StandardDomain.initial_data["anonlabels"]["b2-varindex"] = ("b2-varindex", "")
    return {'version': 0.2}
