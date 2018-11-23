#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
The main module of the Belle II Analysis Software Framework
"""

# import most things to be backwards compatible
from basf2.core import *  # noqa
from basf2.utils import print_params, print_path  # noqa

# that would be all but due to oversights `from basf2 import *` would also make
# the sys and os modules available and we used to have a fw object. So let's
# provide them for compatibility but issue warnings if they are accessed


class ScopeDeprecationWarning():
    """
    Simple wrapper class to allow to issue deprecation warnings if a class
    or scope is used for the first time
    """

    def __init__(self, message, scope):
        """
        Arguments:
            message (str): Message to show the first time any attribute of
                scope is used. The placeholder {key} can be used to print the
                name of the requested attribute
            scope: Object to wrap
        """
        #: Message to be printed
        self.message = message
        #: Object to be wrapped
        self.scope = scope
        #: Did we already print a warning?
        self.warned = False

    def __getattr__(self, key):
        """Print warning on first attribute access"""
        if not self.warned:
            pybasf2.B2WARNING(self.message.format(key=key))
            self.warned = True
        return getattr(self.scope, key)


import sys as _sys
import os as _os
#: wrapper for the sys module as `from basf2 import *` made it available before ...
sys = ScopeDeprecationWarning("Implicit import of sys from basf2 is deprecated and will be removed in release-04. "
                              "To get rid of this warning either\n"
                              "   a) stop using `from basf2 import *` or\n"
                              "   b) add `import sys` after the `from basf2 import *`", _sys)
#: wrapper for the os module as `from basf2 import *` made it available before ...
os = ScopeDeprecationWarning("Implicit import of os from basf2 is deprecated and will be removed in release-04. "
                             "To get rid of this warning either\n"
                             "   a) stop using `from basf2 import *` or\n"
                             "   b) add `import os` after the `from basf2 import *`", _os)
#: wrapper for the framework object which is not necessary anymore
fw = ScopeDeprecationWarning("The Framework object `fw` is deprecated and will be removed in release-04.\n"
                             "   Please use the same functionality directly from the basf2 module.\n"
                             "   So instead of `basf2.fw.{key}` just use `basf2.{key}`",
                             _sys.modules[__name__])

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
