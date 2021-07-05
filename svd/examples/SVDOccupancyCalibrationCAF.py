##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
############################################################
# Python script to test the collector and
# algorithm function to port SVDOCcupancyCalibrations to CAF
#
# Laura Zani (September 2019)
############################################################

import basf2 as b2

import os
import sys
import multiprocessing

from ROOT.Belle2 import SVDOccupancyCalibrationsAlgorithm

from caf.framework import CAF, Calibration, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
# import vertex as vx

b2.set_log_level(b2.LogLevel.INFO)

input_branches = [
    'RawSVDs'
]

b2.set_log_level(b2.LogLevel.INFO)


def SVDOccupancyCalibrations(files, tags):

    # Set-up re-processing path
    path = b2.create_path()

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

    collector = b2.register_module('SVDOccupancyCalibrationsCollector')
#    collector.param("SVDShaperDigitsName", "SVDShaperDigits")
    collector.param("SVDShaperDigitsName", "SVDShaperDigitsZS5")
    algorithm = SVDOccupancyCalibrationsAlgorithm("SVDOccupancyCAF")

    calibration = Calibration('SVDOccupancy',
                              collector=collector,
                              algorithms=algorithm,
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
    input_files = [os.path.abspath(file) for file in [
            "/group/belle2/dataprod/Data/Raw/e0008/r01309/sub00/physics.0008.01309.HLT5.f00098.root"]]
    #  "/group/belle2/dataprod/Data/Raw/e0008/r01309/sub00/physics.0008.01309.HLT5*"]]
    # "/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009/\
    # e0008/4S/r01309/skim/hlt_bhabha/cdst/sub00/cdst.physics.0008.01309.HLT*"]]

    if not len(input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    svdOccupCAF = SVDOccupancyCalibrations(input_files,
                                           ['giulia_CDCEDepToADCConversions_rel4_patch',
                                            'data_reprocessing_prompt_rel4_patch'])
# beamspot.max_iterations = 0

    cal_fw = CAF()
    cal_fw.add_calibration(svdOccupCAF)

    cal_fw.backend = backends.LSF()

# Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
