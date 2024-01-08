#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Descriptor: script to test decay tables

import basf2 as b2
import generators as ge
import sys


decfile = b2.find_file(str(sys.argv[1]))
print(decfile)

b2.set_log_level(b2.LogLevel.ERROR)

main = b2.Path()

main.add_module('EventInfoSetter', evtNumList=[10], expList=[0], runList=[0])

ge.add_evtgen_generator(path=main, finalstate='signal', signaldecfile=decfile)

b2.process(path=main)
print(b2.statistics)
