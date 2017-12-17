#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 2_Reconstruction.py
# description     : Reconstruction cosmic events (CRY generator)
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 7. 11. 2017

# *****************************************************************************

from basf2 import *
from simulation import add_simulation
import tracking
import reconstruction
from modularAnalysis import *
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

inname = "generation.root"
outname = "reconstruction.root"

if len(sys.argv) == 3:
    inname = (sys.argv)[1]
    outname = (sys.argv)[2]

main = create_path()

main.add_module('RootInput', inputFileName=inname)

main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
# main.add_module('Gearbox')

# detector reconstruction
components = [
    'BeamPipe',
    'MagneticField',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM']

"""
    'TOP',
    'ARICH',
    'ECL'
"""

# reconstruction
add_svd_reconstruction(main)
add_pxd_reconstruction(main)
tracking.add_geometry_modules(main, components=components)
tracking.add_ckf_based_track_finding(main, components=components, reco_tracks="RecoTracks")
main.add_module("TrackCreator", recoTrackColName="RecoTracks", useClosestHitToIP=True)
main.add_module('EKLMReconstructor')
main.add_module('BKLMReconstructor')
main.add_module('KLMK0LReconstructor')
main.add_module('Muid')

main.add_module(
    "CDCCosmicTrackMerger",
    recoTracksStoreArrayName="RecoTracks",
    mergedRecoTracksStoreArrayName="CosmicRecoTracks",
    deleteOtherRecoTracks=True)

main.add_module('DAFRecoFitter', recoTracksStoreArrayName='CosmicRecoTracks', resortHits=True)

main.add_module(
    "TrackCreator",
    recoTrackColName="CosmicRecoTracks",
    trackColName="CosmicTracks",
    trackFitResultColName="CosmicTrackFitResults",
    useClosestHitToIP=True)

# main.add_module("FittedTracksStorer",inputRecoTracksStoreArrayName="RecoTracks",outputRecoTracksStoreArrayName="CosmicRecoTracks")

main.add_module('RootOutput', outputFileName=outname)

"""
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)

main.add_module('MillepedeCollector',
                minPValue=0.0,
                components=['VXDAlignment'],
                tracks=['CosmicRecoTracks'],
                particles=[],
                vertices=[],
                primaryVertices=[],
                calibrateVertex=False,
                useGblTree=True)
"""

progress = register_module('ProgressBar')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
