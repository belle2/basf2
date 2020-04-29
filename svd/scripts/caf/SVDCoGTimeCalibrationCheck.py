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
from rawdata import *
from svd.CoGCalibration_utils_checkCalibration import SVDCoGTimeCalibrationCheckModule
from basf2 import conditions
import rawdata as raw

import matplotlib.pyplot as plt
import simulation


def remove_module(path, name):
    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path

localdb = sys.argv[1]
filename = sys.argv[2]
run = sys.argv[3]
exp = sys.argv[4]
# branches = ['SVDShaperDigits', 'SVDShaperDigitsFromTracks', 'EventT0', 'SVDEventInfo']
branches = ['RawSVDs', 'SVDShaperDigitsFromTracks', 'EventT0']

trk_outputFile = "TrackFilterControlNtuples_" + localdb + ".root"
nSVD = 6
nCDC = 1
pVal = 0.0  # 0001

inputFileList = []

if filename.rfind(".root") != -1:
    inputFileList.append(filename)
else:
    with open(filename, 'r') as f:
        inputFileList = [line.strip() for line in f]

conditions.override_globaltags()
conditions.globaltags = [
    "svd_NOCoGCorrections",
    "staging_data_reprocessing_proc11",
    "data_reprocessing_proc11_baseline",
    "online_proc11"]

# conditions.globaltags = [
#     'svd_NOCoGCorrections',
#     'klm_alignment_testing'
# ]

conditions.testing_payloads = [
    str(localdb) + "/database.txt",
]

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileNames', inputFileList)
# rootinput.param('branchNames', branches)
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
# unpack raw data to get SVDEventInfo
add_unpackers(main, components=['SVD'])

# re-reconstruct SVDShaperDigitsFromTracks using the localDB
add_svd_reconstruction(main)

for moda in main.modules():
    if moda.name() == 'SVDCoGTimeEstimator':
        moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
        moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
    if moda.name() == 'SVDSimpleClusterizer':
        moda.param("Clusters", 'SVDClustersFromTracks')
        moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
        moda.param("timeAlgorithm", 0)
    if moda.name() == 'SVDSpacePointCreator':
        moda.param("SVDClusters", 'SVDClustersFromTracks')

main = remove_module(main, 'SVDMissingAPVsClusterCreator')

check = SVDCoGTimeCalibrationCheckModule()
check.set_localdb(localdb)
check.set_run_number(run)
check.set_exp_number(exp)
main.add_module(check)

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

print_path(main)

# Process events
process(main)

print(statistics)
