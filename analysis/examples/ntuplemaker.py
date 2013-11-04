#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################
# Example script to demonstrate the NtupleMaker module
# (inspired by myBtoDpi.py)
#
# Christian Oswald (oswald@physik.uni-bonn.de), Uni Bonn, 2013.
###########################################################

import sys

if len(sys.argv) != 3:
    # the program name and the two arguments
    # stop the program and print an error message
    sys.exit('Must provide two input parameters:[output_root_file_name] [#events_to_generate]'
             )

rootFileName = sys.argv[1]
nOfEvents = int(sys.argv[2])

import os

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()

# ---------------------------------------------------------------
# EvtGen
# boost all momenta to LAB system
# use specified user decay file
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/examples/exampleEvtgenDecayFiles/BtoDpi.dec')

# specify number of events to be generated in job
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [nOfEvents])  # we want to process 'nOfEvents' events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

main.add_module(eventinfosetter)
main.add_module(evtgeninput)

# ---------------------------------------------------------------
# Offical simulation setup
add_simulation(main)

# ---------------------------------------------------------------
# Official reconstruction setup
add_reconstruction(main)

# ---------------------------------------------------------------
# Main part of this script starts here
# First make modular reconstruction of B- -> D0 pi-; D0 -> K- pi+ decay chain
# And then fill nTuple with NtupleMakeModule

# ----> Load final state particles
particleloader = register_module('ParticleLoader')
main.add_module(particleloader)

# ----> Select kaons and pions
kaonselector = register_module('ParticleSelector')
kaonselector.param('PDG', -321)
kaonselector.param('ListName', 'k')
main.add_module(kaonselector)

pionselector = register_module('ParticleSelector')
pionselector.param('PDG', 211)
pionselector.param('ListName', 'pi')
main.add_module(pionselector)

# ----> Reco D0
combinerD0 = register_module('ParticleCombiner')
combinerD0.param('PDG', -421)
combinerD0.param('ListName', 'D0')
combinerD0.param('InputListNames', ['k', 'pi'])
main.add_module(combinerD0)

# ----> Reco B
combinerB = register_module('ParticleCombiner')
combinerB.param('PDG', 521)
combinerB.param('ListName', 'B')
combinerB.param('InputListNames', ['D0', 'pi'])
main.add_module(combinerB)

ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', rootFileName)
# Name of the output TTree
ntuple1.param('strTreeName', 'test1')
# particle list with reconstructed decays
ntuple1.param('strListName', 'B')
# Specifiy output tools
# decay descriptor strings ignored (parser for  not yet implemented)
ntuple1.param('strTools', [
    'EventMetaData',
    'B-',
    'Kinematics',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'MCKinematics',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    ])
main.add_module(ntuple1)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
