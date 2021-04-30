from lxml import etree
from lxml.builder import E
from IPython import get_ipython
from . import core as b2core
import multiprocessing

import ROOT

#: Show an informational message once before the first process() call
_process_warning = False


def _create_pathhtml(path, name="Path", details=None):
    """Create an html fragment for a given Path object"""
    size = len(path.modules())
    fragment = E.div(f"{name} with {size} modules")
    if details is not None:
        fragment.append(details)

    module_list = E.ol(style="list-style-type:decimal; font-style:normal;")
    for m in path.modules():
        name = m.name()
        tname = m.type()
        tname = f" (type \"{tname}\")" if tname != name else ""
        module = E.li("Module ", E.b(name), tname)
        detail_list = E.ul(style="margin:0em 0em 0.5em 0em")
        for p in m.available_params():
            if p.setInSteering:
                detail_list.append(E.li(E.code(f"{p.name} = {p.values}")))
        for c in m.get_all_conditions():
            after = str(c.get_after_path())
            details = E.span(" (condition: ", E.code(f"{c.get_operator()} {c.get_value()}"), ", afterwards:", E.code(after), ')')
            detail_list.append(E.li(_create_pathhtml(c.get_path(), "Conditional Path", details=details),
                                    style="list-style-type: disclosure-closed"))
        if len(detail_list) > 0:
            module.append(detail_list)

        module_list.append(module)
    if len(module_list) > 0:
        fragment.append(module_list)
    return fragment


def print_path_html(path):
    """Print a path (and all sub paths) as html"""
    return etree.tostring(_create_pathhtml(path, "basf2.Path")).decode()


def print_module_html(module):
    """Print a module (and its parameters) as well as the description"""
    name = module.name()
    tname = module.type()
    tname = f" (type {tname})" if tname != name else ""
    module_html = E.div(f"basf2.Module ", E.b(name), tname, E.p(module.description),
                        E.style("table * { text-align: left !important; }", scoped="scoped"))
    cols = ["parameter", "type", "default", "current", "changed", "is required"]
    if len(module.available_params()) > 0:
        params = E.tbody()
        for p in module.available_params():
            params.append(E.tr(E.td(p.name), E.td(E.code(str(p.type))), E.td(E.code(str(p.default))), E.td(E.code(str(p.values))),
                               E.td(E.b("yes") if p.setInSteering else "no"),
                               E.td(E.b("yes") if p.forceInSteering else "no")))
            params.append(E.tr(E.td(), E.td(p.description, colspan="5")))

        module_html.append(E.table(E.thead(E.tr(*[E.th(e) for e in cols])), params))

    return etree.tostring(module_html).decode()


def enable_notebooksupport():
    """
    Small function to enable some notebook compatibility.
    Mostly make sure that log messages are shown correctly (they are C++ so they need some special love to show up).
    """
    # we're in a notebook, reset log system to print to python sys.stdout
    # and we know we want colored output even though stdout might not be a terminal
    b2core.logging.reset()
    b2core.logging.add_console(True)
    b2core.logging.enable_python_logging = True

    # we need to import ROOT because otherwise it can lockup later
    import ROOT

    # register html converters for common objects
    html_formatter = get_ipython().display_formatter.formatters['text/html']
    html_formatter.for_type(b2core.Path, print_path_html)
    html_formatter.for_type(b2core.Module, print_module_html)

# convenience wrap the process() function to use a calculation object


def process(path, max_event=0):
    """
    Start processing events using the modules in the given `basf2.Path` object.

    Can be called multiple times in one steering file.

    This is a convenience wrapper which will automatically call the
    `process()` function in a separate process to prevent FATAL errors from killing
    the notebook.

    Caveats:
        As processing is done in a separate process the global state of the notebook
        cannot be modified. For example a python modules collecting information will appear to be
        empty after execution and global variables will also not be modified.

        If you need to modify the state of the notebook during processing you need to call
        basf2.core.process

    Parameters:
        path: The path with which the processing starts
        max_event:  The maximal number of events which will be processed, 0 for no limit

    Returns:
        None
    """
    global _process_warning
    if(not _process_warning):
        _process_warning = True
        b2core.B2INFO("process() called in a Jupyter Notebook. See help(process) for caveats")
    ctx = multiprocessing.get_context("fork")
    process = ctx.Process(target=b2core.process, args=(path, max_event))
    process.start()
    process.join()
    if process.exitcode != 0:
        raise RuntimeError("Event processing was not successful")
