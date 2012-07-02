#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from basf2 import *


def printAllModules(moduleList):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    for (moduleName, sharedLib) in sorted(moduleList.iteritems()):
        try:
            current_module = register_module(moduleName)
            print_params(current_module, False, sharedLib)
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')


# Get the list of available modules
avModList = fw.list_available_modules()

# If exactly one argument is given, print the specified module.
# Otherwise print all modules.
if len(sys.argv) == 2:
    if sys.argv[1] in avModList:
        current_module = register_module(sys.argv[1])
        print_params(current_module, False, avModList[sys.argv[1]])
    else:
        B2ERROR('Print module information: A module with the name "'
                + sys.argv[1] + '" does not exist!')
else:
    printAllModules(avModList)
