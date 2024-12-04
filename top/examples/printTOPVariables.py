#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------
# Print all available TOP variables
# usage: basf2 top/examples/printTOPVariables.py
# ---------------------------------------------------------------------

from variables import variables
import basf2.utils as b2utils
from ROOT import gSystem

# load top library
gSystem.Load('libtop.so')

allVars = variables.getVariables()
vars = []
for v in allVars:
    vars.append((v.group, v.name, v.description))

rows = []
current_group = ''
for (group, name, description) in vars:
    if 'TOP' in group:
        if current_group != group:
            current_group = group
            rows.append([group])
        rows.append([name, description])

b2utils.pretty_print_description_list(rows)
