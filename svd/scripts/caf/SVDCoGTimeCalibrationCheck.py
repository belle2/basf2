# !/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################################
#
# checks SVD CoG Calibration stored in a localDB
#
# usage: basf2 SVDCoGTimeCalibrationCheck  localDB input_root_file/file_list
# localDB = name of the local DB folder
# input_root_file = single root file with SVDShaperDigitsFromTracks and EventT0
# file_list = text file with a list of rootfiles, one per line
#
#################################################################################


from basf2 import *
from svd import *
import os
import math
from array import array
import basf2
import sys
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVDCoGTimeCalibrations
from svd import *
from svd.CoGCalibration_utils_checkCalibration import SVDCoGTimeCalibrationCheckModule

import matplotlib.pyplot as plt
import simulation

localdb = sys.argv[1]
filename = sys.argv[2]
branches = ['SVDShaperDigits', 'SVDShaperDigitsFromTracks', 'EventT0']

trk_outputFile = "TrackFilterControlNtuples_" + localdb + ".root"
nSVD = 6
nCDC = 1
pVal = 0.0  # 0001


# inputFileList = [
#    "/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009/\
# e0008/4S/r01309/skim/hlt_bhabha/cdst/sub00/cdst.physics.0008.01309.HLT*"
# ]

inputFileList = []

if filename.rfind(".root") != -1:
    inputFileList.append(filename)
else:
    with open(filename, 'r') as f:
        inputFileList = [line.strip() for line in f]


# reset_database()
use_database_chain()
use_local_database(str(localdb) + "/database.txt", str(localdb), invertLogging=True)

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileNames', inputFileList)
rootinput.param('branchNames', branches)
main.add_module(rootinput)

main.add_module("Gearbox")
main.add_module("Geometry", useDB=True)

# Track selection - NOT APPLIED
'''
trkFlt = register_module('TrackFilter')
trkFlt.param('outputFileName', trk_outputFile)
trkFlt.param('outputINArrayName', 'SelectedTracks')
trkFlt.param('outputOUTArrayName', 'ExcludedTracks')
trkFlt.param('min_NumHitSVD', nSVD)
trkFlt.param('min_NumHitCDC', nCDC)
trkFlt.param('min_Pvalue', pVal)
trkFlt.logging.log_level = LogLevel.DEBUG
main.add_module(trkFlt)
'''
# re-reconstruct SVDShaperDigitsFromTracks using the localDB

add_svd_reconstruction(main)

for moda in main.modules():
    if moda.name() == 'SVDCoGTimeEstimator':
        moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
        moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
    if moda.name() == 'SVDSimpleClusterizer':
        moda.param("Clusters", 'SVDClustersFromTracks')
        moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
    if moda.name() == 'SVDSpacePointCreator':
        moda.param("SVDClusters", 'SVDClustersFromTracks')

check = SVDCoGTimeCalibrationCheckModule()
check.set_localdb(localdb)
main.add_module(check)

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

print_path(main)

# Process events
process(main)

print(statistics)
