#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation

set_random_seed(0)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

components = [
    'MagneticFieldConstant4LimitedRCDC',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
]

# FIXME: StoreSecondaries has to be true!
add_simulation(main, components)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'PXDClusterRescueSimulationData_signal_withSecondaries_1000events.root')
# output.param('branchNames', ['PXDClusters', 'MCParticles', 'PXDDigits',
#             'PXDClustersToPXDDigits', 'PXDClustersToMCParticles',
#             'MCParticlesToPXDClusters'])
main.add_module(output)
main.add_module(register_module('ProgressBar'))

process(main)

# Print call statistics
print(statistics)
