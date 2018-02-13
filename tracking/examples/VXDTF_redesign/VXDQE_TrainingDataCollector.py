#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
# VXDTF2 Example Scripts - Collect Data for use with VXDQE_teacher
#
# Uses QETrainingDataCollector module to collect data to be able to
# train a FastBDT weightfile
#
# Contributors: Jonas Wagner, Sebastian Racs
#####################################################################

from basf2 import *
from tracking import add_vxd_track_finding_vxdtf2, add_mc_matcher, add_hit_preparation_modules

estimationMethod = 'tripletFit'
clusterInfo = 'Average'

eval_file_train = "Upsilon4S_ForMVA_10000Events_w16th_overlay_Bkg.root"  # Change to correct training file
sector_map = None  # Default SectorMap

# ---------------------------------------------------------------------------------------
name = 'VXDEQE_CollectedTrainingData_Default'

# Logging and Debug Level
set_log_level(LogLevel.INFO)

path = create_path()

rootInput = register_module('RootInput')
rootInput.param('inputFileName', eval_file_train)
path.add_module(rootInput)

# Event Info Module
eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

path.add_module("Gearbox")
path.add_module("Geometry")

add_hit_preparation_modules(path, components=['SVD'])

add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracks", components=['SVD'], suffix="", sectormap_file=sector_map)

add_mc_matcher(path, components=['SVD'])

data = register_module('QETrainingDataCollector')
data.param('EstimationMethod', estimationMethod)
data.param('ClusterInformation', clusterInfo)
data.param('TrainingDataOutputName', name + '.root')
data.param('SpacePointTrackCandsStoreArrayName', 'SPTrackCands')
data.param('MCRecoTracksStoreArrayName', 'MCRecoTracks')
path.add_module(data)

print_path(path)
process(path)
print(statistics)
