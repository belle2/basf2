#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - Collect Data for use with VXDQE_teacher
#
# Uses VXDQETrainingDataCollector module to collect data to be able to
# train a FastBDT weightfile
#
# Contributors: Jonas Wagner, Sebastian Racs
#####################################################################

import basf2 as b2
from tracking import add_vxd_track_finding_vxdtf2, add_mc_matcher, add_hit_preparation_modules

estimationMethod = 'tripletFit'
clusterInfo = 'Average'

eval_file_train = "Upsilon4S_ForMVA_10000Events_w16th_overlay_Bkg.root"  # Change to correct training file
sector_map = None  # Default SectorMap

# ---------------------------------------------------------------------------------------
name = 'VXDEQE_CollectedTrainingData_Default'

# Logging and Debug Level
b2.set_log_level(b2.LogLevel.INFO)

path = b2.create_path()

rootInput = b2.register_module('RootInput')
rootInput.param('inputFileName', eval_file_train)
path.add_module(rootInput)

# Event Info Module
eventinfoprinter = b2.register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

path.add_module("Gearbox")
path.add_module("Geometry")

add_hit_preparation_modules(path, components=['SVD'])

add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracks", components=['SVD'], suffix="", sectormap_file=sector_map)

add_mc_matcher(path, components=['SVD'])

data = b2.register_module('VXDQETrainingDataCollector')
data.param('EstimationMethod', estimationMethod)
data.param('ClusterInformation', clusterInfo)
data.param('TrainingDataOutputName', name + '.root')
data.param('SpacePointTrackCandsStoreArrayName', 'SPTrackCands')
data.param('MCRecoTracksStoreArrayName', 'MCRecoTracks')
path.add_module(data)

b2.print_path(path)
b2.process(path)
print(b2.statistics)
