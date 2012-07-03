#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import textwrap
from basf2 import *


def printAllModules(moduleList):
    """
    Loop over the list of available modules,
    register them and print their information
    """

    #how many characters of description to print per line?
    try:
        #get terminal width
        columns = int(os.popen('stty size', 'r').read().split()[1])
        if columns < 50:
            columns = 50
    except:
        columns = 80

    for (moduleName, sharedLib) in sorted(moduleList.iteritems()):
        try:
            current_module = register_module(moduleName)
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')

        #use automatic word wrapping on module description
        description = textwrap.wrap(current_module.description(), columns - 22)
        first = True
        for text in description:
            if not first:
                moduleName = ""
            first = False
            print "%-20s  %-s" % (moduleName, text)

    print ''
    print ('------------------------------------------------------'
          + '-----------------------')
    print ''
    print 'To show detailed information on a module, including its parameters,'
    print 'type \'basf2 -m ModuleName\'.'

# Get the list of available modules
avModList = fw.list_available_modules()

# If exactly one argument is given, print the specified module.
# Otherwise print all modules.
if len(sys.argv) == 2:
    if sys.argv[1] in avModList:
        try:
            current_module = register_module(sys.argv[1])
        except:
            B2ERROR('The module could not be loaded. This is most likely '
                    + 'caused by a library with missing links.')
        print_params(current_module, False, avModList[sys.argv[1]])
    else:
        B2ERROR('Print module information: A module with the name "'
                + sys.argv[1] + '" does not exist!')
else:
    printAllModules(avModList)
