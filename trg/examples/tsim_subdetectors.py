#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The example of running simulation of subdetectors trigger.
# basf2 tsim_subdetectors.py -o outputrootfile

import basf2 as b2
from simulation import add_simulation
from L1trigger import add_subdetector_tsim


main = b2.create_path()
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

babayaganlo = b2.register_module('BabayagaNLOInput')
babayaganlo.param('FMax', 7.5e4)
babayaganlo.param('FinalState', 'ee')
babayaganlo.param('MaxAcollinearity', 180.0)
babayaganlo.param('ScatteringAngleRange', [15.0, 165.0])
# babayaganlo.param('VacuumPolarization', 'hadr5')
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('SearchMax', 10000)
babayaganlo.param('VPUncertainty', True)
main.add_module(babayaganlo)

# add simulation
add_simulation(main)

# add trigger simlation of sub detectors (CDC, ECL, and bKLM are included currently)
add_subdetector_tsim(main, component=["CDC", "ECL", "KLM"])

branches = [
    'EventMetaData',
    'TRGCDC2DFinderTracks',
    'TRGCDC2DFitterTracks',
    'TRGCDC3DFitterTracks',
    'TRGCDCNeuroTracks',
    'TRGECLClusters',
    'TRGECLClustersToTRGCDC2DFinderTracks',
    'TRGECLClustersToTRGCDCNeuroTracks',
    'TRGECLTrgs',
    'TRGKLMHits',
    'TRGKLMTracks'
]
branches += ['MCParticles']
# output
main.add_module('RootOutput', branchNames=branches)


# main
b2.process(main)
print(b2.statistics)
# ===<END>
