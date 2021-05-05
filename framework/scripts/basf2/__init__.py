#!/usr/bin/env python3

"""
The main module of the Belle II Analysis Software Framework
"""

# import most things to be backwards compatible
from basf2.core import *  # noqa
from basf2.utils import print_params, print_path  # noqa
from pybasf2 import logging
import sys as _sys

# check for jupyter notebook
_is_ipython = hasattr(__builtins__, '__IPYTHON__') or 'IPython' in _sys.modules
if _is_ipython:
    from IPython import get_ipython as _get_ipython
    _ip = _get_ipython()
    if hasattr(_ip, "kernel"):
        # we're in a notebook, reset log system to print to python sys.stdout
        logging.enable_python_logging = True
        # also in this case we really don't need a log summary
        logging.enable_summary(False)

        # convenience wrap the process() function to use a calculation object
        def process(path, max_event=0):
            """
            Start processing events using the modules in the given `basf2.Path` object.

            Can be called multiple times in one steering file (some restrictions apply:
            modules need to perform proper cleanup & reinitialisation, if Geometry is
            involved this might be difficult to achieve.)

            This is a convenience wrapper which will automatically call the
            `process()` function in a separate process using `hep_ipython_tools`

            Parameters:
                path: The path with which the processing starts
                max_event:  The maximal number of events which will be processed, 0 for no limit

            Returns:
                a `hep_ipython_tools.calculation.Calculation` object
            """

            from hep_ipython_tools.ipython_handler_basf2 import handler as _handler
            calculation = _handler.process(path, max_event=max_event)
            calculation.start()
            calculation.wait_for_end()
            calculation.show_log()
            return calculation
