#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import basf2
from generators import add_evtgen_generator

# interpret the first input argument as decay file to facilitate testing of dec-files
dec_file = None
final_state = 'mixed'
if len(sys.argv) > 1:
    dec_file = os.path.abspath(sys.argv[1])
    final_state = 'signal'
    print("using following decay file: " + dec_file)


# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# EvtGen
add_evtgen_generator(path=main, finalstate=final_state, signaldecfile=dec_file)

# run
main.add_module("Progress")
main.add_module("RootOutput", outputFileName="evtgen_upsilon4s.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
