#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# Get the list of available modules
avModList = fw.list_available_modules()

# Loop over the list of available modules, register them and print their information
for (moduleName, sharedLib) in avModList.iteritems():
    current_module = register_module(moduleName)
    print_params(current_module, False, sharedLib)

