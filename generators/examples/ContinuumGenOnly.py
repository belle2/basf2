#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from generators import add_continuum_generator

# suppress messages and warnings during processing
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# run
main.add_module("Progress")

# use default continuum production
add_continuum_generator(main, finalstate='ccbar')

# add full root output
main.add_module("RootOutput", outputFileName="continuum.root")

# print MC particles (for debugging)
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
