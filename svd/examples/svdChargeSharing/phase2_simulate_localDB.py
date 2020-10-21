#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Simulating BBbar events, geometry created from DB chain
# usage: basf2_phase2_simulate_localDB.py fileOUT localDB_dir
##################################################################################
import os
from basf2 import *
from generators import *

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
main = create_path()

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
process(main)

# show call statistics
print(statistics)
