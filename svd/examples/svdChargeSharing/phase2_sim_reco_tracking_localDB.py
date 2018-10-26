#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Reconstruction of simulated events with geometry created from DB chain
# usage: basf2 phase2_sim_reco_tracking.py fileIN fileOUT localDB_dir
##################################################################################

import os
from basf2 import *
from tracking import *
from svd import *
from rawdata import *
from ROOT import Belle2
from reconstruction import add_reconstruction
import os.path
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

use_database_chain()
# use_central_database("development")
use_local_database(localdb_dir + "/database.txt", localdb_dir)

main = create_path()
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
print_path(main)
main.add_module('ProgressBar')
# Process events
process(main)
print(statistics)
