#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
        # we're in a notebook
        from basf2._notebooksupport import enable_notebooksupport, process
        enable_notebooksupport()
