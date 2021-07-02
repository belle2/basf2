#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Simulating BBbar events with geometry created from xml file
# usage: basf2_phase2_simulate.py fileOUT
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

dec_file = None
final_state = 'mixed'

# main path
main = b2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1002, runList=0, evtNumList=100)

# create geometry from xml file
gearbox = b2.register_module('Gearbox')
geomfile = '/geometry/Beast2_phase2.xml'
if geomfile != 'None':
    gearbox.param('fileName', geomfile)

main.add_module(gearbox)
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['SVD'])
main.add_module(geometry)

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
