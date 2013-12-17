#!/usr/bin/env python
# -*- coding: utf-8 -*-

# addapted from ExtrapCDCTracksToSVD.py
import os
from basf2 import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml, comment the next line out
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])

# shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {  # ---    'momentumParams': [0.4, 1.6],
    'pdgCodes': [13, -13],
    'nTracks': 20,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.5, 1.5],
    'thetaGeneration': 'uniform',
    'thetaParams': [60., 120.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }
pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')
# make simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

si_mctrackfinder = register_module('TrackFinderMCTruth')
si_mctrackfinder.logging.log_level = LogLevel.WARNING

# select which detectors you would like to use
si_mctrackfinder_param = {  # ---        'MinimalNDF': 6,
                            # ---        'UseClusters': 1,
                            # ---        'WhichParticles': ['PXD', 'SVD'], # 'primary'
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'GFTrackCandidatesColName': 'si_mcTracksCand',
    }
        # 'Force2DSVDClusters': 1,
        # 'forceExisting2DClusters4SVD': 0
si_mctrackfinder.param(si_mctrackfinder_param)

cdc_mctrackfinder = register_module('TrackFinderMCTruth')
cdc_mctrackfinder.logging.log_level = LogLevel.WARNING

# select which detectors you would like to use
cdc_mctrackfinder_param = {  # ---        'MinimalNDF': 6,
                             # ---        'UseClusters': 1,
                             # ---        'WhichParticles': ['PXD', 'SVD'], # 'primary'
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'cdc_mcTracksCand',
    }
        # 'Force2DSVDClusters': 1,
        # 'forceExisting2DClusters4SVD': 0
cdc_mctrackfinder.param(cdc_mctrackfinder_param)

# fitting
# -1) fitting the tracks from silicon detectors
si_fitting = register_module('GenFitter')

si_fitting_param = {  # ---    'FilterId': 1,
    'GFTrackCandidatesColName': 'si_mcTracksCand',
    'GFTracksColName': 'si_mcGFTracks',
    'TracksColName': 'si_mcTracks',
    'PDGCodes': [],
    'NIterations': 1,
    'ProbCut': 0.001,
    }
si_fitting.param(si_fitting_param)

# -2) fitting the tracks from cdc detector
cdc_fitting = register_module('GenFitter')

cdc_fitting_param = {  # ---    'FilterId': 1,
    'GFTrackCandidatesColName': 'cdc_mcTracksCand',
    'GFTracksColName': 'cdc_mcGFTracks',
    'TracksColName': 'cdc_mcTracks',
    'PDGCodes': [],
    'NIterations': 1,
    'ProbCut': 0.001,
    }

cdc_fitting.param(cdc_fitting_param)

# track merger
si_cdcTracksMerger = register_module('SiCDCTrackMerger')
trackMeger_param = {  # (in cm) use cdc inner wall
                      #    'CDC_wall_radius':        16.29,  #(in cm) use cdc outer wall
                      # default False
    'SiGFTracksColName': 'si_mcGFTracks',
    'CDCGFTracksColName': 'cdc_mcGFTracks',
    'CDC_wall_radius': 16.25,
    'Theta_min': 0,
    'Theta_max': 3.14159,
    'N_Theta_bin': 6,
    'theta_overlap_size': 0.05,
    'N_Phi_bin': 8,
    'phi_overlap_size': 0.05,
    'produce_root_file': True,
    'root_output_filename': 'si_cdc_trackmerger.root',
    }

si_cdcTracksMerger.param(trackMeger_param)

# output
# ---output = register_module('RootOutput')
# ---output.param('outputFileName', 'SiCDCTrackMerger.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(si_mctrackfinder)
main.add_module(cdc_mctrackfinder)
main.add_module(si_fitting)
main.add_module(cdc_fitting)
main.add_module(si_cdcTracksMerger)
# ---main.add_module(output)

# Process events
process(main)
print statistics
