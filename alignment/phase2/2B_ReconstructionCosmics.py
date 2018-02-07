#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 2_ReconstructionCosmics.py
# description     : Reconstruction cosmic events
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************

from basf2 import *
from simulation import add_simulation
import tracking
import reconstruction
from modularAnalysis import *
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction
from VXDHits import VXDHits

inname = "generation.root"
outname = "reconstruction.root"

if len(sys.argv) == 3:
    inname = (sys.argv)[1]
    outname = (sys.argv)[2]

main = create_path()

main.add_module('RootInput', inputFileName=inname)

main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')

components = [
    'BeamPipe',
    'MagneticField',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM',
    'ECL']

# reconstruction
add_svd_reconstruction(main)
add_pxd_reconstruction(main)
tracking.add_geometry_modules(main, components=components)

tracking.add_cr_tracking_reconstruction(
    main,
    components=components,
    prune_tracks=False,
    skip_geometry_adding=True,
    event_time_extraction=True,
    merge_tracks=False,
    data_taking_period='phase2',
    top_in_counter=False,
    use_second_cdc_hits=False)

main.add_module('Ext')

reconstruction.add_ecl_modules(store, components)
store.add_module('ECLTrackShowerMatch')
store.add_module('ECLElectronId')

main.add_module('EKLMReconstructor')
main.add_module('BKLMReconstructor')
main.add_module('KLMK0LReconstructor')
main.add_module('Muid')
main.add_module('KLMExpert')
main.add_module('ClusterMatcher')

# if magnetic field is in components:
main.add_module(
    "MergerCosmicTracks",
    recoTracksStoreArrayName="RecoTracks",
    MergedRecoTracksStoreArrayName="CosmicRecoTracks",
    usingMagneticField=True)

# if magnetic field is not in components:
"""
main.add_module(
    "MergerCosmicTracks",
    recoTracksStoreArrayName="RecoTracks",
    MergedRecoTracksStoreArrayName="CosmicRecoTracks",
    usingMagneticField=False)
"""

main.add_module('DAFRecoFitter', recoTracksStoreArrayName='CosmicRecoTracks', resortHits=True)

main.add_module(
    "TrackCreator",
    recoTrackColName="CosmicRecoTracks",
    trackColName="CosmicTracks",
    trackFitResultColName="CosmicTrackFitResults",
    useClosestHitToIP=True)

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('MillepedeCollector',
                minPValue=0.0,
                components=['VXDAlignment'],
                tracks=['CosmicRecoTracks'],
                particles=[],
                vertices=[],
                primaryVertices=[],
                calibrateVertex=False,
                useGblTree=True)

main.add_module("CDCDedxPID")

main.add_module('RootOutput', outputFileName=outname)

progress = register_module('ProgressBar')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
