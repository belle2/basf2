#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
# Simulating BBbar events, geometry created from DB chain
# usage: basf2_phase2_simulate_localDB.py fileOUT localDB_dir
##################################################################################
import sys

import basf2 as b2
from generators import add_evtgen_generator

print('***')
print('*** Used steering script:')
with open(sys.argv[0], 'r') as fin:
    print(fin.read(), end="")
print('*** end of the script.')
print('***')

fileOUT = sys.argv[1]
localdb_dir = sys.argv[2]

dec_file = None
final_state = 'mixed'

# main path
main = b2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1002, runList=0, evtNumList=100)

main.add_module('Gearbox')
main.add_module('Geometry')

# EvtGen
add_evtgen_generator(path=main, finalstate=final_state, signaldecfile=dec_file)

# Simulation
main.add_module('FullSim', StoreAllSecondaries=True)

# Digitizer
main.add_module('SVDDigitizer')

main.add_module("Progress")
main.add_module('RootOutput', outputFileName=fileOUT)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
