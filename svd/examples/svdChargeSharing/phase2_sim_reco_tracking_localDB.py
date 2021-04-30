#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Reconstruction of simulated events with geometry created from DB chain
# usage: basf2 phase2_sim_reco_tracking.py fileIN fileOUT localDB_dir
##################################################################################

import basf2 as b2
from tracking import add_tracking_reconstruction
import sys

print('***')
print('*** Used steering script:')
with open(sys.argv[0], 'r') as fin:
    print(fin.read(), end="")
print('*** end of the script.')
print('***')

fileIN = sys.argv[1]
fileOUT = sys.argv[2]
localdb_dir = sys.argv[3]

main = b2.create_path()
main.add_module('RootInput', inputFileName=fileIN)

main.add_module('Gearbox')
main.add_module('Geometry')

# SVD reconstruction
main.add_module('SVDCoGTimeEstimator')
main.add_module('SVDSimpleClusterizer', Clusters="SVDClusters")

# add tracking recontruction
add_tracking_reconstruction(main, components=['SVD'])
# remove cut on CoG in SpacePointCreator
for module in main.modules():
    if module.name() == 'SVDSpacePointCreator':
        module.param("MinClusterTime", -999)

main.add_module('VXDDedxPID', enableDebugOutput=True)

main.add_module(
    'RootOutput',
    outputFileName=fileOUT,
    branchNames=[
        "SVDShaperDigits",
        "SVDClusters",
        "SVDRecoDigits",
        "Tracks",
        "RecoTracks",
        "TracksToRecoTracks",
        "TrackFitResults",
        "MCParticles",
        "SVDTrueHits",
        "TracksToVXDDedxLikelihoods",
        "TracksToVXDDedxTracks",
        "VXDDedxLikelihoods",
        "VXDDedxTracks"])
b2.print_path(main)
main.add_module('ProgressBar')
# Process events
b2.process(main)
print(b2.statistics)
