##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################
# Python script to test the collector and
# algorithm function to port SVDOCcupancyCalibrations to CAF
# usage: basf2 SVDOccupancyAndHotStripsCalibrationsCAF.py \
# -i "/group/belle2/dataprod/Data/Raw/e00<expNum>/r02324<runNum>/sub00/*.HLT*"
############################################################

import basf2 as b2

import os
import sys
import multiprocessing
import datetime
import glob

from ROOT import Belle2, TFile
from ROOT.Belle2 import SVDOccupancyCalibrationsAlgorithm
from ROOT.Belle2 import SVDHotStripsCalibrationsAlgorithm

from caf.framework import CAF, Calibration, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
# import vertex as vx

b2.set_log_level(b2.LogLevel.INFO)

now = datetime.datetime.now()
uniqueID_occup = ''
uniqueID_hotStrips = ''

input_branches = [
    'RawSVDs'
]

b2.set_log_level(b2.LogLevel.INFO)


def SVDOccupancyAndHotStripsCalibrations(files, tags):

    # Set-up re-processing path
    path = b2.create_path()

    # logging.log_level = LogLevel.WARNING

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)
    raw.add_unpackers(path, components=['SVD'])
    path.add_module(
        'SVDZeroSuppressionEmulator',
        SNthreshold=5,
        ShaperDigits='SVDShaperDigits',
        ShaperDigitsIN='SVDShaperDigitsZS5',
        FADCmode=True)

    collector = b2.register_module('SVDOccupancyCalibrationsCollector')
    collector.param("SVDShaperDigitsName", "SVDShaperDigitsZS5")
    algorithm1 = SVDOccupancyCalibrationsAlgorithm(uniqueID_occup)
    algorithm2 = SVDHotStripsCalibrationsAlgorithm(uniqueID_hotStrips)
#    algorithm1 = SVDOccupancyCalibrationsAlgorithm("SVDOccupancyCAF")
#    algorithm2 = SVDHotStripsCalibrationsAlgorithm("SVDHotStripsCAF")

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
    # use by default raw data from cdst of exp12, run 2324 (shaperDigits need to be unpacked, not available in cdst format)
    #    input_files = ["/group/belle2/dataprod/Data/Raw/e0012/r02324/sub00/*"]
    # comment it out to not hardcode the input path:

    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance()
                   .getInputFilesOverride()]
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
                runNum = int(s_run[1:6])
                expNum = int(s_exp[1:5])
    runs.sort()

    firstRun = runs[0]
    lastRun = runs[-1]

    if not len(input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

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

    svdOccupAndHotStripsCAF = SVDOccupancyAndHotStripsCalibrations(input_files,
                                                                   ['data_reprocessing_prompt', 'svd_basic', 'svd_loadedOnFADC'])

    cal_fw = CAF()
    cal_fw.add_calibration(svdOccupAndHotStripsCAF)

    cal_fw.backend = backends.LSF()

# Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
