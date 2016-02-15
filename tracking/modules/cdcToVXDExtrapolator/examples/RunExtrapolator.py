#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Example showing how to run the CDCToVXDExtrapolator. Save the
debugging output for analysis.

"""

from basf2 import process, register_module, statistics, LogLevel
from modularAnalysis import analysis_main, inputMdst

debug = False

# CDCToVXDExtrapolator Options
extrapToDet = True  # Run rextrapolation to the actual detector after finding each hit
searchSens = False  # Include hits from a sensor width away. If false, will cut off based on N sigma (tracking unc.)
allLayers = True  # if True, keep trying to extrapolate through layers even if hits not found
sigma = 3  # Number of sigma within which to accept hits (requires searchSens=False)


components = ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC']
# load input ROOT file, e.g. from running GenerateSimulation.py
inputMdst('exampleInput.root')

# gear = register_module('Gearbox')
# analysis_main.add_module(gear)
geom = register_module('Geometry')
geom.param('components', components)
analysis_main.add_module(geom)

# Until CDCToVXDExtrapolator, the code is just a copy of:
# add_tracking_reconstruction(analysis_main, components, pruneTracks=False)

# Material effects for all track extrapolations
material_effects = register_module('SetupGenfitExtrapolation')
material_effects.param('whichGeometry', 'TGeo')
analysis_main.add_module(material_effects)

# CDC track finder: trasan
cdc_trackcands = 'CDCGFTrackCands'
trackcands = cdc_trackcands
cdc_trackfinder = register_module('Trasan')
cdc_trackfinder.param('GFTrackCandidatesColName', cdc_trackcands)
analysis_main.add_module(cdc_trackfinder)

# VXD track finder
vxd_trackcands = 'VXDGFTrackCands'
vxd_trackfinder = register_module('VXDTF')
vxd_trackfinder.param('GFTrackCandidatesColName', vxd_trackcands)
vxd_trackfinder.param('sectorSetup',
                      ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
                       'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'])
vxd_trackfinder.param('tuneCutoffs', 0.22)
analysis_main.add_module(vxd_trackfinder)

# track merging
track_merger = register_module('MCTrackCandCombiner')
track_merger.param('CDCTrackCandidatesColName', cdc_trackcands)
track_merger.param('VXDTrackCandidatesColName', vxd_trackcands)
track_merger.param('OutputTrackCandidatesColName', 'TrkMerge')
analysis_main.add_module(track_merger)

# Add an intermediate fitting step
trackfitter2 = register_module('GenFitter')
trackfitter2.param("GFTrackCandidatesColName", "TrkMerge")
trackfitter2.param("GFTracksColName", "TrkGFTs")
# trackfitter2.param("StoreFailedTracks", True)
if debug:
    trackfitter2.logging.log_level = LogLevel.DEBUG
    trackfitter2.logging.debug_level = 200
analysis_main.add_module(trackfitter2)

# Add CDCToVXDExtrapolator
add_vxd_hits = register_module('CDCToVXDExtrapolator')
add_vxd_hits.param("GFTrackColName", "TrkGFTs")
add_vxd_hits.param("SearchSensorDimensions", searchSens)
add_vxd_hits.param("ExtrapolateToDetector", extrapToDet)
add_vxd_hits.param("HitNSigmaZ", sigma)
add_vxd_hits.param("HitNSigmaXY", sigma)
add_vxd_hits.param("AllLayers", allLayers)
if debug:
    add_vxd_hits.logging.log_level = LogLevel.DEBUG
    add_vxd_hits.logging.debug_level = 200
# add_vxd_hits.param("UseKalman", True)
add_vxd_hits.param("UseKalman", False)
rname = "06_0518"
if debug:
    rname += "_debug"
if extrapToDet:
    rname += "_wextrap"
else:
    rname += "_woextrap"
if not searchSens:
    rname += "_sig%d" % sigma
if allLayers:
    rname += "_allL"
# If RootOutputFilename is not set, no debugging output file is generated
add_vxd_hits.param("RootOutputFilename", "root/" + rname + ".root")
analysis_main.add_module(add_vxd_hits)

# And build tracks from output of CDCToVXDExtrapolator
trackBuilder = register_module("TrackBuilder")
analysis_main.add_module(trackBuilder)

# Show progress of processing
progress = register_module('Progress')
analysis_main.add_module(progress)

# Show display
display = register_module('Display')
# analysis_main.add_module(display)

process(analysis_main)
print(statistics)
