#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates an ntuple for validation
# purposes
# Author Benjamin Oberhof
########################################################


import os
import random
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [20])
eventinfoprinter = register_module('EventInfoPrinter')

# Fixed random seed
set_random_seed(123456)

# EvtGen generator
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

# shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate with pGun
param_pGun = {
    'pdgCodes': [13],
    'nTracks': 3,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0, 5],
    'thetaGeneration': 'uniform',
    'thetaParams': [15., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
}
pGun.param(param_pGun)

# trackfinders
vxd_mctrackfinder = register_module('TrackFinderMCTruth')
vxd_mctrackfinder.logging.log_level = LogLevel.WARNING
vxd_mctrackfinder_param = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'VXDGFTrackCands'
}
vxd_mctrackfinder.param(vxd_mctrackfinder_param)

vxd_trackfinder = register_module('VXDTF')
vxd_trackfinder.param('GFTrackCandidatesColName', 'VXDGFTrackCands')
vxd_trackfinder.param('sectorSetup',
                      ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
                       ])
vxd_trackfinder.param('tuneCutoffs', 0.22)
# path.add_module(vxd_trackfinder)

cdc_mctrackfinder = register_module('TrackFinderMCTruth')
cdc_mctrackfinder.logging.log_level = LogLevel.WARNING
cdc_mctrackfinder_param = {
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'CDCGFTrackCands',
}
cdc_mctrackfinder.param(cdc_mctrackfinder_param)

cdc_trackfinder = register_module('Trasan')
cdc_trackfinder.logging.log_level = LogLevel.WARNING
cdc_trackfinder_param = {'GFTrackCandidatesColName': 'CDCGFTrackCands'}
cdc_trackfinder.param(cdc_trackfinder_param)

# MERGING
# track merger
vxd_cdcTracksMerger = register_module('VXDCDCTrackMerger')
trackMerger_param = {
    'VXDGFTracksColName': 'VXDGFTracks',
    'CDCGFTracksColName': 'CDCGFTracks',
    'VXDGFTrackCandsColName': 'VXDGFTrackCands',
    'CDCGFTrackCandsColName': 'CDCGFTrackCands',
    #    'TrackCandColName': 'TrackCands',
    'relMatchedTracks': 'MatchedTracksIdx',
    'chi2_max': 100,
    'merge_radius': 2.0,
    'recover': 1,
}
vxd_cdcTracksMerger.param(trackMerger_param)

# track merger analysis
vxd_cdcMergerAnalysis = register_module('VXDCDCMergerAnalysis')
trackMergerAnalysis_param = {
    'VXDGFTracksColName': 'VXDGFTracks',
    'CDCGFTracksColName': 'CDCGFTracks',
    'VXDGFTrackCandsColName': 'VXDGFTrackCands',
    'CDCGFTrackCandsColName': 'CDCGFTrackCands',
    'root_output_filename': 'VXDCDCMergerAnalysis.root',
}
vxd_cdcMergerAnalysis.param(trackMergerAnalysis_param)
vxd_cdcMergerAnalysis.logging.log_level = LogLevel.DEBUG


display = register_module('Display')
display.param('useClusters', True)
display.param('showTrackLevelObjects', True)
display.param('options', 'DHMPS')
# display.param(display_param)

# ...MCPrinter...
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.param('maxLevel', -1)

# match the found track candidates with MCParticles
# cdcmcmatching = register_module('CDCMCMatching')
# param_cdcmcmatching = {'GFTrackCandidatesColName': 'CDCTrackCands'}
# cdcmcmatching.param(param_cdcmcmatching)

mctf = register_module('TrackFinderMCTruth')
mctf.param('GFTrackCandidatesColName', 'MCGFTrackCands')

matcher1 = register_module('MCTrackMatcher')
# matcher1.param('MCGFTrackCandsColName','MCGFTrackCands')
matcher1.param('PRGFTrackCandsColName', 'VXDGFTrackCands')

matcher2 = register_module('MCTrackMatcher')
# matcher2.param('MCGFTrackCandsColName','MCGFTrackCands')
matcher2.param('PRGFTrackCandsColName', 'CDCGFTrackCands')

VXDtrackFitter = register_module('GenFitter')
VXDtrackFitter.param('GFTrackCandidatesColName', 'VXDGFTrackCands')
VXDtrackFitter.param("PDGCodes", [211])
VXDtrackFitter.param('GFTracksColName', 'VXDGFTracks')
VXDtrackFitter.set_name('VXD-only')

CDCtrackFitter = register_module('GenFitter')
CDCtrackFitter.param('GFTrackCandidatesColName', 'CDCGFTrackCands')
CDCtrackFitter.param("PDGCodes", [211])
CDCtrackFitter.param('GFTracksColName', 'CDCGFTracks')
CDCtrackFitter.set_name('CDC-only')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
# main.add_module(gearbox)
# main.add_module(geometry)
main.add_module(pGun)
# main.add_module(mcparticleprinter)
# main.add_module(evtgeninput)
add_simulation(main)
# main.add_module(vxd_mctrackfinder)
# main.add_module(vxd_trackfinder)
# main.add_module(cdc_mctrackfinder)
# main.add_module(cdc_trackfinder)
# main.add_module(setupgen)
# main.add_module(VXDtrackFitter)
# main.add_module(CDCtrackFitter)
# main.add_module(vxd_cdcTracksMerger)
add_reconstruction(main)
main.add_module(mctf)
main.add_module(matcher1)
main.add_module(matcher2)
main.add_module(vxd_cdcMergerAnalysis)
# main.add_module(display)

# Process events
process(main)
print(statistics)
