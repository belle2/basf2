#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
The main module of the Belle II Analysis Software Framework
"""

# import most things to be backwards compatible
from basf2.core import *  # noqa
from basf2.utils import print_params, print_path  # noqa
import sys as _sys

#: bool indicating whether we're running in an ipython instance
_is_ipython = hasattr(__builtins__, '__IPYTHON__') or 'IPython' in _sys.modules
if _is_ipython:
    from IPython import get_ipython as _get_ipython
    # check if we're in a notebook
    if hasattr(_get_ipython(), "kernel"):
        # import dedicated process function to override core version for notebooks
        from basf2._notebooksupport import enable_notebooksupport, process  # noqa
        enable_notebooksupport()
