#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use VXDCDCTrackMerger
# and VXDCDCMergerAnalysis modules.
#
# Example steering file - 2014 Belle II Collaboration
#
# Author: Benjamin Oberhof, benjamin.oberhof@lnf.infn.it
#
########################################################

import os
import random
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from simulation import add_simulation

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# EvtGen generator
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

# shoot particles in the detector
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13],
    'nTracks': 1,
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

# trackfinders
si_mctrackfinder = register_module('TrackFinderMCTruth')
si_mctrackfinder.logging.log_level = LogLevel.WARNING
si_mctrackfinder_param = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'VXDTracksCand',
}
# 'Force2DSVDClusters': 1,
# 'forceExisting2DClusters4SVD': 0
si_mctrackfinder.param(si_mctrackfinder_param)

vxd_trackfinder = register_module('VXDTF')
vxd_trackfinder.param('GFTrackCandidatesColName', 'VXDTracksCand')
vxd_trackfinder.param('sectorSetup',
                      ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
                       ])
vxd_trackfinder.param('tuneCutoffs', 0.22)

cdc_mctrackfinder = register_module('TrackFinderMCTruth')
cdc_mctrackfinder.logging.log_level = LogLevel.WARNING

# select which detectors you would like to use
cdc_mctrackfinder_param = {
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'CDCTracksCand',
}
cdc_mctrackfinder.param(cdc_mctrackfinder_param)

cdc_trackfinder = register_module('Trasan')
cdc_trackfinder.logging.log_level = LogLevel.WARNING
cdc_trackfinder_param = {'GFTrackCandidatesColName': 'CDCTracksCand'}
cdc_trackfinder.param(cdc_trackfinder_param)

cand_merger = register_module('TrackCandMerger')
cand_merger_param = {'SiTrackCandidatesColName': 'VXDTracksCand',
                     'CDCTrackCandidatesColName': 'CDCTracksCand',
                     'TrackCandidatesCollection': 'TracksCand'}
cand_merger.param(cand_merger_param)

# Material effects for all track extrapolations
material_effects = register_module('SetupGenfitExtrapolation')

# fitting the tracks
fitting = register_module('GenFitter')
fitting_param = {
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
trackMerger_param = {
    'VXDGFTracksColName': 'VXDTracks',
    'CDCGFTracksColName': 'CDCTracks',
    'relMatchedTracks': 'MatchedTracksIdx',
    'chi2_max': 100,
    'recover': 1
}

vxd_cdcTracksMerger.param(trackMerger_param)


# track merger analysis
vxd_cdcMergerAnalysis = register_module('VXDCDCMergerAnalysis')
trackMergerAnalysis_param = {
    'VXDGFTracksColName': 'VXDTracks',
    'CDCGFTracksColName': 'CDCTracks',
    'GFTracksColName': 'GFTracks',
    'TrackCandColName': 'TracksCand',
    'UnMergedCands': 'UnMergedCand',
    'root_output_filename': 'VXD_CDC_1000Evts.root',
}
vxd_cdcMergerAnalysis.param(trackMergerAnalysis_param)
vxd_cdcMergerAnalysis.logging.log_level = LogLevel.DEBUG


display = register_module('Display')
display.param('useClusters', True)
display.param('showTrackLevelObjects', True)
display.param('options', 'DHMPS')


mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.param('maxLevel', -1)

mctf = register_module('TrackFinderMCTruth')
mctf.param('GFTrackCandidatesColName', 'MCGFTrackCands')

matcher1 = register_module('MCTrackMatcher')
matcher1.param('PRGFTrackCandsColName', 'VXDTracksCand')

matcher2 = register_module('MCTrackMatcher')
matcher2.param('PRGFTrackCandsColName', 'CDCTracksCand')


# Create path
main = create_path()

# Add modules to path
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(pGun)
add_simulation(main)
# main.add_module(si_mctrackfinder)
main.add_module(vxd_trackfinder)
main.add_module(cdc_trackfinder)
# main.add_module(cdc_mctrackfinder)
main.add_module(mctf)
main.add_module(matcher1)
main.add_module(matcher2)
main.add_module(cand_merger)
# main.add_module(material_effects)
main.add_module(fitting)
main.add_module(track_splitter)
main.add_module(vxd_cdcTracksMerger)
main.add_module(vxd_cdcMergerAnalysis)
# main.add_module(display)

# Process events
process(main)
print(statistics)
