#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
import os
import reconstruction
from CosmicAnalysis import CosmicAnalysis
import svd
import pxd

from ROOT import Belle2
import ROOT

inname = "simulated.root"
outname = "reconstructed.root"
dqmname = "dqm.root"

if len(sys.argv) == 4:
    inname = sys.argv[1]
    outname = sys.argv[2]
    dqmname = sys.argv[3]

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
# geometry.param('components', ['PXD', 'SVD'])
# geometry.param('useDB', False)

# create paths
main = create_path()

main.add_module('RootInput', inputFileName=inname)
main.add_module(gearbox)
main.add_module(geometry)

pxd.add_pxd_reconstruction(main)
main.add_module('PXDSpacePointCreator', SpacePoints='PXDSpacePoints')

main.add_module('SVDCoGTimeEstimator')
main.add_module('SVDSimpleClusterizer')

spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param("MinClusterTime", -999)
main.add_module(spCreatorSVD)

track_finder = register_module('TrackFinderVXDCosmicsStandalone')
track_finder.param('SpacePointTrackCandArrayName', "")
track_finder.param('SpacePoints', ['SVDSpacePoints', 'PXDSpacePoints'])
track_finder.param('QualityCut', 0.0001)
track_finder.param('MinSPs', 3)
track_finder.param('MaxRejectedSPs', 20)
main.add_module(track_finder)

converter = register_module('SPTC2RTConverter')
converter.param('recoTracksStoreArrayName', 'RecoTracks')
main.add_module(converter)

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
# main.add_module('KalmanRecoFitter', pdgCodesToUseForFitting=[13])
main.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])
main.add_module(
    'TrackCreator',
    recoTrackColName="RecoTracks",
    trackColName="Tracks",
    trackFitResultColName="TrackFitResults",
    useClosestHitToIP=True,
    useBFieldAtHit=True)


# CosmicAnalysis = CosmicAnalysis(Belle2.Environment.Instance().getOutputFileOverride())
CosmicAnalysis = CosmicAnalysis(dqmname)
main.add_module(CosmicAnalysis)

main.add_module('RootOutput', outputFileName=outname)

main.add_module('ProgressBar')
# main.add_module('Progress')

process(main)
print(statistics)
