#!/usr/bin/env python
# -*- coding: utf-8 -*-

# adapted from ExtrapCDCTracksToVXD.py
import os
import random
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [500])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml, comment the next line out
geometry.param('components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'ECL',
    ])

# EvtGen generator
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

# shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate with pGun
param_pGun = {  # ---    'momentumParams': [0.4, 1.6],
                # fixed, uniform
    'pdgCodes': [13, -13],
    'nTracks': 10,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0, 5],
    'thetaGeneration': 'uniform',
    'thetaParams': [0., 180.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
    }
pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')
# make simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')
cdcDigitizer.logging.log_level = LogLevel.ERROR
pxdDigitizer = register_module('PXDDigitizer')
pxdDigitizer.logging.log_level = LogLevel.ERROR
svdDigitizer = register_module('SVDDigitizer')
svdDigitizer.logging.log_level = LogLevel.ERROR
pxdClusterizer = register_module('PXDClusterizer')
pxdClusterizer.logging.log_level = LogLevel.ERROR
svdClusterizer = register_module('SVDClusterizer')
svdClusterizer.logging.log_level = LogLevel.ERROR

# trackfinders
si_mctrackfinder = register_module('TrackFinderMCTruth')
si_mctrackfinder.logging.log_level = LogLevel.WARNING
# select which detectors you would like to use
si_mctrackfinder_param = {  # ---        'MinimalNDF': 6,
                            # ---        'UseClusters': 1,
                            # ---        'WhichParticles': ['PXD', 'SVD'], # 'primary'
                            # 'MinimalNDF': 5,
                            # 'UseClusters': 1,
                            # 'WhichParticles': ['PXD', 'SVD'],
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'GFTrackCandidatesColName': 'VXDTracksCand',
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
    'GFTrackCandidatesColName': 'CDCTracksCand',
    }
        # 'Force2DSVDClusters': 1,
        # 'forceExisting2DClusters4SVD': 0
cdc_mctrackfinder.param(cdc_mctrackfinder_param)

cand_merger = register_module('TrackCandMerger')
cand_merger_param = {'SiTrackCandidatesColName': 'VXDTracksCand',
                     'CDCTrackCandidatesColName': 'CDCTracksCand',
                     'TrackCandidatesCollection': 'TracksCand'}
cand_merger.param(cand_merger_param)

# fitting
# -1) fitting the tracks from silicon detectors
# si_fitting = register_module('GenFitter')
# si_fitting_param = {  # ---    'FilterId': 1,
#    'GFTrackCandidatesColName': 'mcTracksCand',
#    'GFTracksColName': 'si_mcGFTracks',
    # 'TracksColName': 'si_mcTracks',
    # 'PDGCodes': [],
#    'NMaxIterations': 10,
#    'ProbCut': 0.001,
#    }
# si_fitting.param(si_fitting_param)

# -2) fitting the tracks
fitting = register_module('GenFitter')
fitting_param = {  # ---    'FilterId': 1,
                   # 'MCParticlesColName': 'mcParticlesTruth',
                   # 'TracksColName': 'cdc_mcTracks',
                   # 'PDGCodes': [],
    'GFTrackCandidatesColName': 'TracksCand',
    'GFTracksColName': 'GFTracks',
    'NMaxIterations': 10,
    'ProbCut': 0.001,
    }
fitting.logging.log_level = LogLevel.WARNING
fitting.param(fitting_param)

track_splitter = register_module('GFTrackSplitter')
track_splitter_param = {
    'SiGFTracksColName': 'VXDTracks',
    'CDCGFTracksColName': 'CDCTracks',
    'GFTracksColName': 'GFTracks',
    'storeTrackCandName': 'TracksCand',
    }
# 'CDCHitColName' :
track_splitter.param(track_splitter_param)

# MERGING
# track merger
vxd_cdcTracksMerger = register_module('VXDCDCTrackMerger')
trackMerger_param = {  # (in cm) use cdc inner wall
                       #    'CDC_wall_radius':        16.29,  #(in cm) use cdc outer wall
                       # default False
                       # 'MCParticlesColName': 'mcParticlesTruth',
                       #    'mergedCDCGFTracksColName': 'mergedCDC_Tracks',
                       #    'mergedVXDGFTracksColName': 'mergedSi_Tracks',
                       #    'CDC_wall_radius': 16.25,
                       #    'GFTracksColName': 'GFTracks',
    'VXDGFTracksColName': 'VXDTracks',
    'CDCGFTracksColName': 'CDCTracks',
    'TrackCandColName': 'TracksCand',
    'relMatchedTracks': 'MatchedTracksIdx',
    'chi2_max': 100,
    }
#    'root_output_filename': 'VXD_CDC_trackmerger_test.root',

vxd_cdcTracksMerger.param(trackMerger_param)

# MERGING ANALYSIS
# track merger analysis
vxd_cdcTracksMergerAnalysis = register_module('VXDCDCTrackMergerAnalysis')
trackMergerAnalysis_param = {  # (in cm) use cdc inner wall
                               #    'CDC_wall_radius':        16.29,  #(in cm) use cdc outer wall
                               # default False
                               # 'MCParticlesColName': 'mcParticlesTruth',
                               #    'CDC_wall_radius': 16.25,
    'VXDGFTracksColName': 'VXDTracks',
    'CDCGFTracksColName': 'CDCTracks',
    'GFTracksColName': 'GFTracks',
    'TrackCandColName': 'TracksCand',
    'UnMergedCands': 'UnMergedCand',
    'root_output_filename': 'VXD_CDC_trackmerger_test.root',
    'chi2_max': 100,
    'merge_radius': 2.0,
    }
vxd_cdcTracksMergerAnalysis.param(trackMergerAnalysis_param)
vxd_cdcTracksMergerAnalysis.logging.log_level = LogLevel.DEBUG


class HighlighterModule(Module):

    """Select all things related to a Particle"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj('DisplayData').registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj('DisplayData')
        displayData.create()

        merged = Belle2.PyStoreArray('MergedTracks')
        for p in merged:
            # if abs(p.getPDGCode()) == 413:
            B2WARNING('Highlighting merged tracks')
            displayData.obj().select(p)
                # daughters = p.getFinalStateDaughters()
                # for d in daughters:
                    # selecting the MCParticle also gets the tracks
                    # mcp = d.getRelated('MCParticles')
                    # displayData.obj().select(mcp)
            break  # only one


display = register_module('Display')
display.param('useClusters', True)
display.param('showTrackLevelObjects', True)
    # 'showTrackCandidates' : True)
display.param('options', 'DHMPS')
# display.param(display_param)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
# main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(cdcDigitizer)
main.add_module(si_mctrackfinder)
main.add_module(cdc_mctrackfinder)
main.add_module(cand_merger)
# main.add_module(si_fitting)
main.add_module(fitting)
main.add_module(track_splitter)
main.add_module(vxd_cdcTracksMerger)
main.add_module(vxd_cdcTracksMergerAnalysis)
# main.add_module(HighlighterModule())
# main.add_module(display)

# ---main.add_module(output)

# Process events
process(main)
print statistics
