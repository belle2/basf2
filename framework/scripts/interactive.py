#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Defines a function embed() that can be used to drop into an interactive python
shell from within a steering file or python module. If available, IPython will
be used. Use Ctrl+D to exit the shell.

Usage:
import interactive
interactive.embed()
"""

from basf2 import *

try:  # current IPython version
    import IPython
    embed = IPython.embed
except:  # older IPython version
    try:
        from IPython.Shell import IPShellEmbed
        embed = IPShellEmbed()
    except:  # no ipython at all
        import sys
        import code

        def embed():
            """
            Drop into interactive python shell.
            """

            # no predefined function that does everything for us, so we
            # have to get local and global variables from the caller:
            stack_depth = 0
            call_frame = sys._getframe(stack_depth).f_back
            local_ns = call_frame.f_locals
            global_ns = call_frame.f_globals

            code.interact(local=dict(global_ns, **local_ns))
