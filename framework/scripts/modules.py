#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from basf2 import *


# Get the list of available modules
avModList = fw.list_available_modules()

# If exactly one argument is given, print the specified module.
# Otherwise print all modules.
if len(sys.argv) == 2:
    if sys.argv[1] in avModList:
        try:
            current_module = register_module(sys.argv[1])
            print_params(current_module, False, avModList[sys.argv[1]])
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')
    else:
        B2ERROR('Print module information: A module with the name "'
                + sys.argv[1] + '" does not exist!')
else:
    print_all_modules(avModList)
