#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Simulating BBbar events
# usage: basf2_phase2_simulate_localDB.py fileOUT localDB_dirs
##################################################################################
import os
from basf2 import *
from generators import *

fileOUT = str(sys.argv[1])
localdb_dir = str(sys.argv[2])

# use_database_chain()
# use_central_database("development")
# use_local_database(localdb_dir+"database.txt", localdb_dir)

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
