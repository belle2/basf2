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
