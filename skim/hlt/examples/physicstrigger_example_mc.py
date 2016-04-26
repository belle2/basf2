#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################################
#
# This file shows how to execute high level trigger (hlt) module
# The data flow isGenerator-->Simulation-->Level3 trigger-->Reconstruction->Physics trigger
#
# For reconstruction module, events are reconstructed with information form all
# detectors except PXD
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
from modularAnalysis import generateY4S
from HLTTrigger import add_HLT_Y4S
from modularAnalysis import generateY4S
from ROOT import Belle2

logging.log_level = LogLevel.INFO
emptypath = create_path()


# generation of 100 events according to the specified DECAY table
# Y(4S) -> Btag- Bsig+
# Btag- -> D0 pi-; D0 -> K- pi+
# Bsig+ -> mu+ nu_mu
#
# generateY4S function is defined in analysis/scripts/modularAnalysis.py
generateY4S(100, Belle2.FileSystem.findFile('analysis/examples/tutorials/B2A101-Y4SEventGeneration.dec'))

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

# physics trigger
add_HLT_Y4S(analysis_main)


# outputs
branches = ['HLTTags', 'L3Tags', 'L3Tracks', 'L3Clusters',
            'PhysicsTriggerInformations']

output = register_module('RootOutput')
output.param('outputFileName', 'output_physicstrigger.root')
output.param('branchNames', branches)
analysis_main.add_module(output)

process(analysis_main)

print(statistics)
