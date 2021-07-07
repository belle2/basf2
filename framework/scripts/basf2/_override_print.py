#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This is a small module to replace the builtin print function with one that
flushes by default.

This is necessary as we mix a lot of C++ and python output and this means we
need to flush the output frequently otherwise the order of lines is totally
garbled

It overrides the function in the builtins module so this will propagate to
everywhere. We do this in a extra module to be able to check the python version
on `import basf2`.
"""

import builtins


def flush_print(*args, **argk):
    """print function which flushes by default"""
    argk.setdefault("flush", True)
    builtins._print(*args, **argk)


# remember original print function
builtins._print = print
# and replace it with the flushing one
builtins.print = flush_print
