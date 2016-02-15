#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates an ntuple for validation
# purposes
# Author Benjamin Oberhof
########################################################

"""
<header>
<output>VXDCDCMergerSinglePartTruthFinder.root</output>
<contact>Benjamin Oberhof, tracking@belle2.kek.jp</contact>
</header>
"""

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
eventinfosetter.param('evtNumList', [10000])
eventinfoprinter = register_module('EventInfoPrinter')

# Fixed random seed
set_random_seed(123456)

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
    #    'ECL',
])

# EvtGen generator
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

# shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate with pGun
param_pGun = {  # ---    'momentumParams': [0.4, 1.6],
                # fixed, uniform
                # [0, 5],
    'pdgCodes': [13],
    'nTracks': 1,
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
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'VXDGFTrackCands',
}
# 'Force2DSVDClusters': 1,
# 'forceExisting2DClusters4SVD': 0
si_mctrackfinder.param(si_mctrackfinder_param)

vxd_trackfinder = register_module('VXDTF')
vxd_trackfinder.param('GFTrackCandidatesColName', 'VXDGFTrackCands')
# if components is not None and 'PXD' not in components:
#    vxd_trackfinder.param('sectorSetup',
#                          ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-moreThan500MeV_SVD',
#                          'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-125to500MeV_SVD',
#                          'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-30to125MeV_SVD'])
#    vxd_trackfinder.param('tuneCutoffs', 0.06)
# else:
vxd_trackfinder.param('sectorSetup',
                      ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
                       ])
vxd_trackfinder.param('tuneCutoffs', 0.22)
# path.add_module(vxd_trackfinder)

cdc_mctrackfinder = register_module('TrackFinderMCTruth')
cdc_mctrackfinder.logging.log_level = LogLevel.WARNING

# select which detectors you would like to use
cdc_mctrackfinder_param = {  # ---        'MinimalNDF': 6,
                             # ---        'UseClusters': 1,
                             # ---        'WhichParticles': ['PXD', 'SVD'], # 'primary'
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    'GFTrackCandidatesColName': 'CDCGFTrackCands',
}
# 'Force2DSVDClusters': 1,
# 'forceExisting2DClusters4SVD': 0
cdc_mctrackfinder.param(cdc_mctrackfinder_param)

cdc_trackfinder = register_module('Trasan')
cdc_trackfinder.logging.log_level = LogLevel.WARNING
cdc_trackfinder_param = {'GFTrackCandidatesColName': 'CDCGFTrackCands'}  # 'UseCDCHits': 1,
# 'UseSVDHits': 0,
# 'UsePXDHits': 0,
# 'Force2DSVDClusters': 1,
# 'forceExisting2DClusters4SVD': 0
cdc_trackfinder.param(cdc_trackfinder_param)

cand_merger = register_module('TrackCandMerger')
cand_merger_param = {'SiTrackCandidatesColName': 'VXDGFTrackCands',
                     'CDCTrackCandidatesColName': 'CDCGFTrackCands',
                     'TrackCandidatesCollection': 'GFTrackCands'}
cand_merger.param(cand_merger_param)

# fitting
# -1) fitting the tracks from silicon detectors
# si_fitting = register_module('GenFitter')
# si_fitting_param = {  # ---    'FilterId': 1,
#    'GFTrackCandidatesColName': 'mcTrackCands',
#    'GFTracksColName': 'si_mcGFTracks',
# 'TracksColName': 'si_mcTracks',
# 'PDGCodes': [],
#    'NMaxIterations': 10,
#    'ProbCut': 0.001,
#    }
# si_fitting.param(si_fitting_param)

setupgen = register_module('SetupGenfitExtrapolation')

# -2) fitting the tracks
fitting = register_module('GenFitter')
fitting_param = {  # ---    'FilterId': 1,
                   # 'MCParticlesColName': 'mcParticlesTruth',
                   # 'TracksColName': 'cdc_mcTracks',
                   # 'PDGCodes': [],
    'GFTrackCandidatesColName': 'GFTrackCands',
    'GFTracksColName': 'GFTracks',
    'NMaxIterations': 10,
    'ProbCut': 0.001}
fitting.logging.log_level = LogLevel.WARNING
fitting.param(fitting_param)

track_splitter = register_module('GFTrackSplitter')
track_splitter_param = {
    'SiGFTracksColName': 'VXDGFTracks',
    'CDCGFTracksColName': 'CDCGFTracks',
    'GFTracksColName': 'GFTracks',
    'storeTrackCandName': 'GFTrackCands',
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
    'VXDGFTracksColName': 'VXDGFTracks',
    'CDCGFTracksColName': 'CDCGFTracks',
    #    'TrackCandColName': 'GFTrackCands',
    'relMatchedTracks': 'MatchedTracksIdx',
    'chi2_max': 100,
    'merge_radius': 2.0,
    'recover': 1,
}
#    'root_output_filename': 'VXD_CDC_trackmerger_test.root',

vxd_cdcTracksMerger.param(trackMerger_param)

# MERGING ANALYSIS
# track merger analysis
vxd_cdcMergerAnalysis = register_module('VXDCDCMergerAnalysis')
trackMergerAnalysis_param = {  # (in cm) use cdc inner wall
                               #    'CDC_wall_radius':        16.29,  #(in cm) use cdc outer wall
                               # default False
                               # 'MCParticlesColName': 'mcParticlesTruth',
                               #    'CDC_wall_radius': 16.25,
    'VXDGFTracksColName': 'VXDGFTracks',
    'CDCGFTracksColName': 'CDCGFTracks',
    'VXDGFTrackCandsColName': 'VXDGFTrackCands',
    'CDCGFTrackCandsColName': 'CDCGFTrackCands',
    # 'GFTracksColName': 'GFTracks',
    # 'TrackCandColName': 'GFTrackCands',
    # 'UnMergedCands': 'UnMergedCand',
    'root_output_filename': '../VXDCDCMergerSinglePartTruthFinder.root',
    #    'chi2_max': 100,
    #    'merge_radius': 2.0,
}
vxd_cdcMergerAnalysis.param(trackMergerAnalysis_param)
vxd_cdcMergerAnalysis.logging.log_level = LogLevel.DEBUG


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


# display = register_module('Display')
# display.param('useClusters', True)
# display.param('showTrackLevelObjects', True)
# 'showTrackCandidates' : True)
# display.param('options', 'DHMPS')
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
# main.add_module(g4sim)
# main.add_module(pxdDigitizer)
# main.add_module(pxdClusterizer)
# main.add_module(svdDigitizer)
# main.add_module(svdClusterizer)
# main.add_module(cdcDigitizer)
add_simulation(main)
main.add_module(si_mctrackfinder)
# main.add_module(vxd_trackfinder)
main.add_module(cdc_mctrackfinder)
# main.add_module(cdc_trackfinder)
# main.add_module(cdcmcmatching)
main.add_module(mctf)
main.add_module(matcher1)
main.add_module(matcher2)
# main.add_module(cand_merger)
main.add_module(setupgen)
# main.add_module(si_fitting)
# main.add_module(fitting)
# main.add_module(track_splitter)
main.add_module(VXDtrackFitter)
main.add_module(CDCtrackFitter)
main.add_module(vxd_cdcTracksMerger)
main.add_module(vxd_cdcMergerAnalysis)
# main.add_module(HighlighterModule())
# main.add_module(display)

# ---main.add_module(output)

# Process events
process(main)
print(statistics)
