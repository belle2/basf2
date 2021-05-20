#!/usr/bin/env python3

"""
The main module of the Belle II Analysis Software Framework
"""

# import most things to be backwards compatible
from basf2.core import *  # noqa
from basf2.utils import print_params, print_path  # noqa
from pybasf2 import logging
import sys as _sys

#: bool indicating whether we're running in an ipython instance
_is_ipython = hasattr(__builtins__, '__IPYTHON__') or 'IPython' in _sys.modules
if _is_ipython:
    from IPython import get_ipython as _get_ipython
    if hasattr(_get_ipython(), "kernel"):
        # we're in a notebook
        from basf2._notebooksupport import enable_notebooksupport, process
        enable_notebooksupport()
