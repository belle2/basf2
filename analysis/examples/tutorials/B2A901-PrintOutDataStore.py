#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to print out the
# contents of the DataStore for each event.
# This is useful for debugging purposes, e.g. to check if
# the MC files contain the neccessary (MDST) StoreArrays.
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (September 2018)
#
######################################################

import basf2 as b2
import modularAnalysis as ma

#  check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B2pi0D_D2hh_D2hhh_B2munu.root'):
    b2.B2FATAL("You need the example data installed. Run `b2mount-tutorial-cloud` in terminal for it.")

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2pi0D_D2hh_D2hhh_B2munu.root',
             path=my_path)

# print out the contents of the DataStore
ma.printDataStore(path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
