#!/usr/bin/env python
# -*- coding: utf-8 -*-

################################################################################################
#
# This file shows how to execute high level trigger (hlt) module
# The data flow isGenerator-->Simulation-->Level3 trigger-->Reconstruction->Physics trigger
#
# For reconstruction module, events are reconstructed with information form all
# detectors except PXD
#
# For PhyscisTriger module, to be flexible, the user custom option is open.
# If you want to select the events by using yourself selection criteria,
# just set UserCustomOpen to a value larger than zero as commented sentences below,
# and provide your selection criteria to UserCustomCut.
#
# physicstrigger.param('UserCustomOpen',1); (default: 0)
# physicstrigger.param('UserCustomCut','2<=nTracksHLT<=10'); (default: empty)
#
# Please pay attention that only variables defined in VariableManager/PhysicsTriggerVariables{h,cc}
# could be used in your selection criteria.
# If you want to use other variables defined by yourself, please define it in PhysicsTriggerVariables.cc firstly.
# Once user custom is open, the official selection criteria will be closed automaticly.
#
#
# Contributor: Chunhua LI
#
##################################################################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import generateY4S

logging.log_level = LogLevel.INFO
emptypath = create_path()

evtmetagen = register_module('EventInfoSetter')
evtmetagen.param('evtNumList', [10])  # we want to process 10 events
evtmetagen.param('runList', [1])  # from run number 1
evtmetagen.param('expList', [1])  # and experiment number 1
analysis_main.add_module(evtmetagen)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
analysis_main.add_module(evtgeninput)

# detecor simulation
components_sim = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
    ]

# simulation
add_simulation(analysis_main, components_sim)

# Level3 trigger
level3 = register_module('Level3')

#  cut value for the energy sum of ECL clusters (default:4.0GeV)
# level3.param('min_energy', 3.0);

#  cut value of dz for reconstructed tracks (default:4.0cm)
# level3.param('dz_cut', 2.0);

#  minimum number of good tracks (default:1)
# level3.param('min_n_trks', 2);

#  save_data=2: save HLTTag, L3Tag, L3Tracks and L3Clusters (default)
#  save_data=1: save HLTTag and L3Tag
#  save_data=0: save only HLTTag
# level3.param('save_data', 0);

analysis_main.add_module(level3)

#  uncomment if you really want to turn on Level-3 filtering
# level3.if_false(emptypath)

# The events are reconstruced with all detectors except PXD
components_rec = [  # 'PXD',
    'MagneticField',
    'BeamPipe',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
    ]

# reconstruction
add_reconstruction(analysis_main, components_rec)

# load all final state Particles
loadReconstructedParticles()

# create charged tracks list
fillParticleList('pi+:HLT', '')

# create gamma list
fillParticleList('gamma:HLT', '')

physicstrigger = register_module('PhysicsTrigger')
# physicstrigger.param('UserCustomOpen',1);
# physicstrigger.param('UserCustomCut','2<=nTracksHLT<=10');
analysis_main.add_module(physicstrigger)

# uncomment if you want to turn on the physics-trigger
# emptypath = create_path()
# physicstrigger.if_false(emptypath)

# outputs
branches = ['HLTTags', 'L3Tags', 'L3Tracks', 'L3Clusters',
            'PhysicsTriggerInformations']

output = register_module('RootOutput')
output.param('outputFileName', 'output_physicstrigger.root')
output.param('branchNames', branches)
analysis_main.add_module(output)

process(analysis_main)

print statistics
