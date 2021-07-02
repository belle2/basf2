#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##################################################################################
# Reconstruction of simulated events, geometry created from xml file
# usage: basf2 phase2_sim_reco_tracking.py fileIN fileOUT
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

main = b2.create_path()
main.add_module('RootInput', inputFileName=fileIN)

gearbox = b2.register_module('Gearbox')
geomfile = '/geometry/Beast2_phase2.xml'
if geomfile != 'None':
    gearbox.param('fileName', geomfile)

main.add_module(gearbox)
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['SVD'])
main.add_module(geometry)

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
