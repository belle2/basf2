#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------
#          Import basf2 environment
# -----------------------------------------------
from basf2_env import *

# -----------------------------------------------
#    Import global basf2 function definitions
# -----------------------------------------------
from basf2_def import *

# -----------------------------------------------------
# replace print function with a flushing print function
# -----------------------------------------------------

import builtins


def flush_print(*args, **argk):
    """print function which flushes by default"""
    argk.setdefault("flush", True)
    builtins._print(*args, **argk)

# remember original print function
builtins._print = print
# and replace it with the flushing one
builtins.print = flush_print
