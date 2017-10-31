#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
import pdg
import glob
import sys

if len(sys.argv) < 3:
    sys.exit("Expected args: magCharge elCharge mass")

mag = float(sys.argv[1])
el = float(sys.argv[2])
mass = float(sys.argv[3])

num_events = 100

main = create_path()
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)

pdg.add_particle('monopole',       99666, mass, 1.0,       el, 0.0)
pdg.add_particle('anti-monopole', -99666, mass, 1.0,      -el, 0.0)
# add_particle  (name,           pdgCode, mass, width, charge, spin, max_width=None, lifetime=0, pythiaID=0)

# generate events
pairgen = register_module('PairGen')
pairgen.param('pdgCode', 99666)
main.add_module(pairgen)

# Phase 2 geometry
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
main.add_module(gearbox)

geometry = register_module('Geometry')
main.add_module(geometry)

# detector simulation
g4sim = register_module('FullSim')
string = ' '
string += str(mag)
string += ' '
string += str(el)
string += ' '
string += str(mass)
string += ' GeV'
g4sim.param('MonopoleDefinition', string)
g4sim.param('UICommands', '/testex/fld/setField 1.5 T')
g4sim.param('trajectoryStore', 1)
main.add_module(g4sim)

add_simulation(main)
add_reconstruction(main)

# Save output of simulation
output = register_module('RootOutput')  # Set output filename
output.param('outputFileName', 'MonopolePair.root')
main.add_module(output)

process(main)
print(statistics)
