#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()
l3exit = create_path()

# specify number of events to be generated in job
evtmetagen = register_module('EventInfoSetter')
evtmetagen.param('evtNumList', [10])  # we want to process 10 events
evtmetagen.param('runList', [1])  # from run number 1
evtmetagen.param('expList', [1])  # and experiment number 1
main.add_module(evtmetagen)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# detector simulation
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'ECL',
    'BKLM',
    'EKLM',
    'STR',
]
add_simulation(main, components)

# Level3 trigger module
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

main.add_module(level3)

#  uncomment if you really want to turn on Level-3 filtering
# level3.if_false(l3exit)

# uncomment if you want to run reconstruction code
add_reconstruction(main, components)

# outputs
branches = [
    'CDCHits',
    'ECLHits',
    'ECLDigits',
    'MCParticles',
    'MCParticlesToCDCHits',
    'MCParticlesToECLHits',
    'HLTTags',
    'L3Tags',
    'L3Tracks',
    'L3Clusters',
]

branches += [
    'Tracks',
    'TrackFitResults',
    'PIDLikelihoods',
    'TracksToPIDLikelihoods',
    'ECLShowers',
    'TracksToECLShowers',
    'ECLGammas',
    'ECLGammasToECLShowers',
    'ECLPi0s',
    'ECLPi0sToECLGammas',
    'EKLMK0Ls',
    'MCParticlesToTracks',
    'ECLShowersToMCParticles',
]

output = register_module('RootOutput')
output.param('outputFileName', 'output.root')
output.param('branchNames', branches)
main.add_module(output)

process(main)

print(statistics)
