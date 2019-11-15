############################################################
# Python script to test the collector and
# algorithm function to port SVDOCcupancyCalibrations to CAF
#
# Laura Zani (September 2019)
############################################################

from basf2 import *

set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing
import datetime
import glob

import ROOT
from ROOT import Belle2, TFile
from ROOT.Belle2 import SVDOccupancyCalibrationsAlgorithm
from ROOT.Belle2 import SVDHotStripsCalibrationsAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
# import vertex as vx

now = datetime.datetime.now()

input_branches = [
    'RawSVDs'
]

set_log_level(LogLevel.INFO)


def SVDOccupancyAndHotStripsCalibrations(files, tags):

    # Set-up re-processing path
    path = create_path()

    # logging.log_level = LogLevel.WARNING

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    # fil = register_module('SVDShaperDigitsFromTracks')
    # fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
    # main.add_module(fil)

    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # with possibly changed global tags
    raw.add_unpackers(path, components=['SVD'])
    path.add_module(
        'SVDZeroSuppressionEmulator',
        SNthreshold=5,
        ShaperDigits='SVDShaperDigits',
        ShaperDigitsIN='SVDShaperDigitsZS5',
        FADCmode=True)

    collector = register_module('SVDOccupancyCalibrationsCollector')
    collector.param("SVDShaperDigitsName", "SVDShaperDigitsZS5")
    algorithm1 = SVDOccupancyCalibrationsAlgorithm("SVDOccupancyCAF")
    algorithm2 = SVDHotStripsCalibrationsAlgorithm("SVDHotStripsCAF")

    calibration = Calibration('SVDOccupancyAndHotStrips',
                              collector=collector,
                              algorithms=[algorithm1, algorithm2],
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    calibration.strategies = strategies.SequentialRunByRun

    return calibration


if __name__ == "__main__":
    # use by default raw data from cdst of exp8, run1309 (shaperDigits need to be unpacked, not available in cdst format)
    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance()
                   .getInputFilesOverride()]
    # [            "/group/belle2/dataprod/Data/Raw/e0008/r01309/sub00/physics.0008.01309.HLT5.f00098.root"]]
    #  "/group/belle2/dataprod/Data/Raw/e0008/r01309/sub00/physics.0008.01309.HLT5*"]]
    # "/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009/\
    # e0008/4S/r01309/skim/hlt_bhabha/cdst/sub00/cdst.physics.0008.01309.HLT*"]]

    print(" ")
    print("INPUT FILES")
    print(" ")
    print(input_files)
    print(" ")

    good_input_files = []
    runs = []
    expNum = int()
    for i in input_files:
        file_list = glob.glob(i)
        for f in file_list:
            tf = TFile.Open(f)
            tree = tf.Get("tree")
            if tree.GetEntries() != 0:
                good_input_files.append(f)
                print(str(f))
                inputStringSplit = f.split("/")
                s_run = str(inputStringSplit[7])
                s_exp = str(inputStringSplit[6])
                print(str(s_run) + " " + str(s_exp))
                runNum = runs.append(int(s_run[1:6]))
                expNum = int(s_exp[1:5])

    runs.sort()

    firstRun = runs[0]
    lastRun = runs[-1]

    if not len(input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    svdOccupAndHotStripsCAF = SVDOccupancyAndHotStripsCalibrations(input_files,
                                                                   ['data_reprocessing_prompt_rel4_patchb',
                                                                    'data_reprocessing_proc10'])
    uniqueID_occup = "SVDOccupancyCalibrations_" + \
        str(now.isoformat()) + "_INFO:_ZS5_Exp" + str(expNum) + "_runsFrom" + \
        str(firstRun) + "to" + str(lastRun)
    print("UniqueID for SVDccupancyCalibrations")
    print("")
    print(str(uniqueID_occup))
    print("")

    uniqueID_hotStrips = "SVDHotStripsCalibrations_" + \
        str(now.isoformat()) + "_INFO:_ZS5_base=-1_relOccPrec=5_absOccThr=0.2_Exp"\
        + str(expNum) + "_runsFrom" + str(firstRun) + "to" + str(lastRun)
    print("UniqueID for SVDHotStripsCalibrations")
    print("")
    print(str(uniqueID_hotStrips))
    print("")

    cal_fw = CAF()
    cal_fw.add_calibration(svdOccupAndHotStripsCAF)

    cal_fw.backend = backends.LSF()

# Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
