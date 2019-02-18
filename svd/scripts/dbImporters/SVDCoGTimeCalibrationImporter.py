# !/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################################
#
# evaluates the CoG corrections, create a localDB
# with the corrections and a root file to check
# the corrections
#
# usage: basf2 SVDCoGTimeCalibratinWithErrorImporter localDB filename
# localDB = name of the local DB folder
# filename = single root file, or file with the list of reconstructed files
#
# this script can be launched with launch_calibration_cog.sh in the
# B2SVD project, svd_CoGHitTime_calibration repository
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
from svd.CoGCalibration_utils import SVDCoGTimeCalibrationImporterModule

import matplotlib.pyplot as plt
import simulation

localdb = sys.argv[1]
filename = sys.argv[2]
branches = ['SVDShaperDigits', 'SVDRecoDigits', 'SVDClusters', 'EventT0', 'Tracks', 'RecoTracks']

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

reset_database()
use_database_chain()
use_central_database('data_reprocessing_proc7')
if '_CHECK' not in localdb:
    use_central_database('svd_NOCoGCorrections')
    if os.path.isdir(localdb):
        print('WARNING, you are using CoG corrections in ' + localdb + ' exiting')
        sys.exit(1)
    else:
        use_local_database(localdb + "/database.txt", localdb, invertLogging=True)
else:
    localdb = localdb.split('_')
    if not os.path.isdir(localdb[0]):
        print('WARNING, the localDB ' + localdb[0] + ' containing the corrections is missing, exiting.')
        sys.exit(1)
    else:
        use_local_database(localdb[0] + "/database.txt", localdb[0], invertLogging=True)
    localdb = sys.argv[1]
    if os.path.isdir(localdb):
        print('WARNING, you are using CoG corrections in ' + localdb + ' exiting')
        sys.exit(1)
    else:
        use_local_database(localdb + "/database.txt", localdb, invertLogging=True)

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileNames', inputFileList)
rootinput.param('branchNames', branches)
main.add_module(rootinput)

main.add_module("Gearbox")
main.add_module("Geometry", useDB=True)

# Track selection - NOT YET
trkFlt = register_module('TrackFilter')
trkFlt.param('outputFileName', trk_outputFile)
trkFlt.param('outputINArrayName', 'SelectedTracks')
trkFlt.param('outputOUTArrayName', 'ExcludedTracks')
trkFlt.param('min_NumHitSVD', nSVD)
trkFlt.param('min_NumHitCDC', nCDC)
trkFlt.param('min_Pvalue', pVal)
# trkFlt.logging.log_level = LogLevel.DEBUG
# main.add_module(trkFlt)

fil = register_module('SVDShaperDigitsFromTracks')
fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
main.add_module(fil)

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

calib = SVDCoGTimeCalibrationImporterModule()
calib.set_localdb(localdb)
calib.notApplyCorrectForCDCLatency(False)  # False = apply correction, True = not apply correction
main.add_module(calib)

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

print_path(main)

# Process events
process(main)

print(statistics)
