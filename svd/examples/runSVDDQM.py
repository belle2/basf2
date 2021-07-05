#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
#
# Test script to produce SVD DQM plots
#
# Author:  Giulia Casarosa
#
# Usage: basf2 runEventT0DQM.py -i "<path/to/file.root>"
#
# 2020 Belle II Collaboration
#############################################################

import basf2 as b2
from basf2 import conditions as b2conditions
from tracking import add_tracking_reconstruction
from rawdata import add_unpackers

# needed for some temporary issues with BKLMDisplacement payload
b2conditions.override_globaltags()
b2conditions.globaltags = ['klm_alignment_testing', 'online']

# main main
main = b2.create_path()

# RAW
files = [' /group/belle2/dataprod/Data/Raw/e0010/r04295/sub00/physics.0010.04295.HLT1*.root']

# old-format cDST
# files=["/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_hadron/cdst/sub00/cdst.physics.0010.04295.HLT1*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_bhabha/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_mumu_2trk/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root"]

# read input rootfile
# -> can be overwritten with the -i option
main.add_module("RootInput", inputFileNames=files)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="SVDDQMOutput.root")

# nee to know SVD geometry to create histograms
main.add_module('Gearbox')
main.add_module('Geometry')

# if using RAW data you need to unpack them
add_unpackers(main, components=['SVD', 'CDC'])
# you may also want to do SVD reconstruction
# add_svd_reconstruction(main)
# or TRACKING reconstruction
add_tracking_reconstruction(main, components=['SVD', 'CDC'])

# if using cDST with new format you have to do svd reconstruction
# add_svd_reconstruction(main)

# add offline ZS
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)


# ** SVD DATA FORMAT - available only with unpacking
# -> it needs SVDDAQDiagnostic
# unpacker = main.add_module('SVDUnpackerDQM')
# unpacker.set_log_level(LogLevel.DEBUG)  # LogLevel.DEBUG / LogLevel.INFO
# unpacker.set_debug_level(100)

# ** SVD ExpressReco General
# main.add_module('SVDDQMExpressReco', offlineZSShaperDigits='SVDShaperDigitsZS5')

# ** SVD Efficiency - available only with full reconstruction
# -> it neeeds Tracks and relations
# main.add_module('SetupGenfitExtrapolation')
# main.add_module('SVDROIFinder', recoTrackListName='RecoTracks', SVDInterceptListName='SVDIntercepts')
# main.add_module('SVDDQMEfficiency')

# ** SVD Clusters On Track - available only with full reconstruction
# -> it neeeds Tracks and relations
# main.add_module('SVDDQMClustersOnTrack')

# ** SVD Occupancy after Injection - need RawFTSW & Offline ZS
# injection = main.add_module('SVDDQMInjection', ShaperDigits='SVDShaperDigitsZS5')
# injection.set_log_level(LogLevel.DEBUG)  # LogLevel.DEBUG / LogLevel.INFO
# injection.set_debug_level(30)

# ** SVD Hit Time
SVDHitTimeDQMmodule = main.add_module("SVDDQMHitTime")
# SVDHitTimeDQMmodule.set_log_level(LogLevel.INFO)  # LogLevel.DEBUG / LogLevel.INFO
# SVDHitTimeDQMmodule.set_debug_level(21)

# == Show progress
main.add_module('Progress')

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
