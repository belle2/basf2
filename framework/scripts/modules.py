#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from basf2 import *


# Get the list of available modules
avModList = fw.list_available_modules()

if len(sys.argv) == 2:
    # If exactly one argument is given, print the specified module.
    modName = sys.argv[1]
    if modName in avModList:
        try:
            current_module = register_module(modName)
            print_params(current_module, False, avModList[modName])
        except:
            B2FATAL('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')
    elif modName == modName.lower():
        # lower case? might be a package instead
        print_all_modules(avModList, modName)
    else:
        B2FATAL('Print module information: A module with the name "'
                + modName + '" does not exist!')
else:
    # Otherwise print all modules.
    print_all_modules(avModList)
