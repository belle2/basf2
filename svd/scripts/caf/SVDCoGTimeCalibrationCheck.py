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


import basf2 as b2
import sys
from svd import add_svd_reconstruction
from svd.CoGCalibration_utils_checkCalibration import SVDCoGTimeCalibrationCheckModule
import rawdata as raw


localdb = sys.argv[1]
filename = sys.argv[2]
run = sys.argv[3]
exp = sys.argv[4]
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

b2.conditions.override_globaltags()
b2.conditions.globaltags = [
    "svd_NOCoGCorrections",
    "staging_data_reprocessing_proc11",
    "data_reprocessing_proc11_baseline",
    "online_proc11"]

b2.conditions.testing_payloads = [
    str(localdb) + "/database.txt",
]

main = b2.create_path()

rootinput = b2.register_module('RootInput')
rootinput.param('inputFileNames', inputFileList)
rootinput.param('branchNames', branches)
main.add_module(rootinput)

main.add_module("Gearbox")
main.add_module("Geometry", useDB=True)

# unpack raw data to get SVDEventInfo
raw.add_unpackers(main, components=['SVD'])

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

main = b2.remove_module(main, 'SVDMissingAPVsClusterCreator')

check = SVDCoGTimeCalibrationCheckModule()
check.set_localdb(localdb)
check.set_run_number(run)
check.set_exp_number(exp)
main.add_module(check)

# Show progress of processing
progress = b2.register_module('ProgressBar')
main.add_module(progress)

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
