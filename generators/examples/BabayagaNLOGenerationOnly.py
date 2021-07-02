#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

########################################################
# 100 radiative Bhabha events are generated using the BabayagaNLO
# generator
#
# Example steering file
########################################################

import basf2
from generators import add_babayaganlo_generator


# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Register the BABAYAGA.NLO module
add_babayaganlo_generator(path=main, finalstate='ee')

# Create the main path and add the modules
main.add_module("Progress")
main.add_module("RootOutput", outputFileName="babayaganlo_ee.root")

main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
