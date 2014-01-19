#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Example steering file for reconstruction of Testbeam data.
#
# Note that due to the way the SVD mapping works, you must execute
# this steering file WHILE IN YOUR MAIN basf2 DIRECTORY, i.e. your
# command line should look somtehing like this:
#   basf2 testbeam/vxd/examples/example_reco_script.py
#
# Please

import os
import sys

from basf2 import *
from setup_vxdtf import setup_vxdtf

######################################################################
#
# configuration

set_log_level(LogLevel.WARNING)

# field off
# fieldOn = True
fieldOn = False

# flag to use if the PXD is in place and should be used in the Reconstruction
havePXD = True

# data file
inputFileName = '~/Dropbox/Public/daq/data/RawData_0000_000220.root'

# histogram file
histoFileName = 'histofile_000220.root'

# which geometry
if havePXD:
    geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'
else:
    geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v1.xml'

# enable genfit visualization?
useGenfitVis = False

# enable standard event viewer?
useEventViewer = False

#####################################################################
#
# the script proper starts here.

# Load Geometry module
gearbox = register_module('Gearbox')
# Telescopes, magnetic field, PXD, SVD
# Need to select the
gearbox.param('fileName', geometry_file)

geometry = register_module('Geometry')
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

if havePXD:
    PXDUnpack = register_module('PXDUnpacker')
    PXDUnpack.param('DHHCmode', True)
    # PXDUnpack.param('HeaderEndianSwap',False);
    PXDUnpack.param('HeaderEndianSwap', True)

    # PXD rawhit converter
    PXDSort = register_module('PXDRawHitSorter')
    # If the same pixel appears again in data, merge the two by adding
    # their charges, or discard the following occurrences?  Setting to
    # True may lead to pixels with over-range charge when there are
    # several repetitions of data.
    PXDSort.param('mergeDuplicates', True)
    # True is conservative with respect to downstream processing chain
    # in case some parts do not support PXD frames. There is always a
    # single frame, though information on number of frames and start
    # rows is propagated.
    PXDSort.param('mergeFrames', True)

    # PXD clusterizer
    PXDClust = register_module('PXDClusterizer')

    # PXD DQM modules
    PXD_DQM = register_module('PXDDQM')
    raw_pxd_dqm = register_module('pxdRawDQM')

# SVD unpacker
SVDUnpack = register_module('SVDUnpacker')
SVDUnpack.param('xmlMapFileName', 'testbeam/vxd/data/SVD-OnlineOfflineMap.xml')

# SVD clusterizer
SVDClust = register_module('SVDClusterizer')
if fieldOn:
    SVDClust.param('TanLorentz_holes', 0.052)
    SVDClust.param('TanLorentz_electrons', 0.)
else:
    SVDClust.param('TanLorentz_holes', 0.)
    SVDClust.param('TanLorentz_electrons', 0.)

# SVD DQM module
SVD_DQM = register_module('SVDDQM')

# Depending on whether we have the PXD, we include it in trackfinding or don't
if havePXD:
    vxdtf = setup_vxdtf('caTracks', ['testBeamMini6GeVVXD-moreThan1500MeV_VXD'
                        ])
else:
    vxdtf = setup_vxdtf('caTracks', ['testBeamMini6GeVSVD-moreThan1500MeV_SVD'
                        ])

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.DEBUG
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
# trackfitter.param('FilterId', 'simpleKalman')
if useGenfitVis:
    trackfitter.param('StoreFailedTracks', True)
trackfitter.param('UseClusters', True)
trackfitter.param('NMinIterations', 3)
trackfitter.param('NMaxIterations', 3)

trackfit_dqm = register_module('TrackfitDQM')
trackfit_dqm.param('GFTrackCandidatesColName', 'caTracks')

# create the main path
main = create_path()

# Add input module.  Since we redo the full reconstruction, we don't
# read a few StoreArrays instead regenerating them on the fly.
input = register_module('RootInput')
input.param('inputFileName', inputFileName)
input.param('excludeBranchNames', [
        'SVDClusters',
        'SVDDigits',
        'SVDClustersToSVDDigits',
        'TrackCands',
        'TrackFitResults',
        'Tracks',
        ])
main.add_module(input)

hman = register_module('HistoManager')
hman.param('histoFileName', histoFileName)
main.add_module(hman)

# Add Progress
progress = register_module('Progress')
main.add_module(progress)

# Add Elapsed Time
elapsed = register_module('ElapsedTime')
elapsed.param('EventInterval', 10000)
main.add_module(elapsed)

# Add Gearbox and geometry
main.add_module(gearbox)
main.add_module(geometry)

if havePXD:
    # PXD: sorter, clusterizer, dqm
    main.add_module(PXDUnpack)
    main.add_module(PXDSort)
    main.add_module(PXDClust)
    main.add_module(PXD_DQM)
    main.add_module(raw_pxd_dqm)

# Add SVD Unpacker
main.add_module(SVDUnpack)

# Add SVD clusterizer
main.add_module(SVDClust)

# Add SVD DQM module
main.add_module(SVD_DQM)

main.add_module(vxdtf)
main.add_module(trackfitter)
main.add_module(vxdtf_dqm)
main.add_module(trackfit_dqm)

if useEventViewer:
    display = register_module('Display')
    display.param('fullGeometry', True)
    main.add_module(display)
if useGenfitVis:
    gfvis = register_module('GenfitVis')
    gfvis.param('onlyBadTracks', 0)
    main.add_module(gfvis)

# Run
process(main)
