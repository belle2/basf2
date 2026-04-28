##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Script to perform the svd hot-strips calibration.
It identifies the hot strips using the criteria in defined in SVDHotStripsCalibrationsAlgorithm.
This calibration is run manually on pedestal runs (runs without beams and detector on PEAK).

b2caf-prompt-run Local config.json inputs.json --permissive

The location of the steering file caf_svd_hotstrips.py must be specified in
the json configuration file, config.json, e.g.

{"caf_script": "/home/belle2/lgcorona/basf2_2/svd/scripts/caf/caf_svd_hotstrips.py", ...}
'''

import basf2 as b2

import sys
import datetime

from ROOT.Belle2 import SVDHotStripsCalibrationsAlgorithm

from caf.framework import Calibration
from caf import strategies
from caf.utils import IoV
from prompt import CalibrationSettings

import rawdata as raw

b2.set_log_level(b2.LogLevel.INFO)

now = datetime.datetime.now()

input_branches = ['RawSVDs']

settings = CalibrationSettings(name="caf_svd_hotstrips",
                               expert_username="lgcorona",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["beam"],
                               expert_config={
                                   "snrThreshold": 5,
                                   "computeAverageOccupancyPerChip": False,
                                   "relativeOccupancyThreshold": 5.0,
                                   "absoluteOccupancyThreshold": 0.2,
                                   "skipHLTRejectedEvents": False
                               })


def create_collector(name="SVDOccupancyCalibrationsCollector",
                     svdShaperDigits="SVDShaperDigits",
                     skipHLTRejectedEvents=False):
    """
    Simply creates a SVDOccupancyCalibrationsCollector module with some options.

    Parameters:
        name: name of the collector to run
        svdShaperDigits: name of the SVDShaperDigits store object to use as input

    Returns:
        pybasf2.Module
    """

    collector = b2.register_module('SVDOccupancyCalibrationsCollector')
    collector.param("SVDShaperDigitsName", svdShaperDigits)
    collector.param("skipHLTRejectedEvents", skipHLTRejectedEvents)

    return collector


def create_algorithm(unique_id,
                     computeAvgOccupancyPerChip=False,
                     relativeOccupancyThreshold=5.,
                     absoluteOccupancyThreshold=0.2):
    """
    Simply creates a SVDHotStripsCalibrationsAlgorithm class.

    Parameters:
        unique_id: name of the uniqueID written in the payload to identify it
        computeAvgOccupancyPerChip: true, compute the average occupancy per chip;
                                    false, compute the average occupancy per sensor/side
        relativeOccupancyThreshold: relative occupancy threshold wrt the average sensor/side
                                    or chip occupancy to define a strip as hot
        absoluteOccupancyThreshold: absolute occupancy threshold to define a strip as hot

    Returns:
        ROOT.Belle2.SVDHotStripsCalibrationAlgorithm
    """

    algorithm = SVDHotStripsCalibrationsAlgorithm(unique_id)
    algorithm.computeAverageOccupancyPerChip(computeAvgOccupancyPerChip)
    algorithm.setRelativeOccupancyThreshold(relativeOccupancyThreshold)
    algorithm.setAbsoluteOccupancyThreshold(absoluteOccupancyThreshold)

    return algorithm


def crate_svd_zerosuppression(svdShaperDigits="SVDShaperDigits",
                              svdShaperDigitsOutput="SVDShaperDigitsZS",
                              snrThreshold=5,
                              fadcMode=True):
    """
    Simply creates a SVDZeroSuppressionEmulator module with some options.

    Parameters:
        svdShaperDigits: name of the input SVDShaperDigits store object
        svdShaperDigitsOutput: name of the output SVDShaperDigits store object.
                               Kept shaper digits are selected with snrThreshold
        snrThreshold: signal-to-noise ratio treshold to select strips
        fadcMode: if true, the approximation to integer for the minimum signal-to-noise
                  ratio (int(snrThreshold*strip_noise + 0.5)) is done. This is the same algorithm
                  applied on the FADC

    Returns:
        pybasf2.Module
    """

    svdZSemulator = b2.register_module("SVDZeroSuppressionEmulator")
    svdZSemulator.param("SNthreshold", snrThreshold)
    svdZSemulator.param("ShaperDigits", svdShaperDigits)
    svdZSemulator.param("ShaperDigitsIN", svdShaperDigitsOutput)
    svdZSemulator.param("FADCmode", fadcMode)

    return svdZSemulator


def create_pre_collector_path(svdShaperDigits="SVDShaperDigits",
                              svdShaperDigitsOutput="SVDShaperDigitsZS",
                              snrThreshold=5,
                              fadcMode=True):
    """
    Create a basf2 path that runs the unpacker and zero suppression emulator.

    Returns:
        pybasf2.Path
    """

    # Set-up re-processing path
    path = b2.create_path()
    path.add_module('Progress')

    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)
    raw.add_unpackers(path, components=['SVD'])
    svdZSemulator = crate_svd_zerosuppression(svdShaperDigits=svdShaperDigits,
                                              svdShaperDigitsOutput=svdShaperDigitsOutput,
                                              snrThreshold=snrThreshold,
                                              fadcMode=fadcMode)
    path.add_module(svdZSemulator)

    return path


def get_calibrations(input_data, **kwargs):
    """
    Simply creates the calibrations to be run in the caf, and returns a list of calibrations.

    Parameters:
        input_data: input_data defined in the json file

    Returns:
        pybasf2.list
    """

    file_to_iov = input_data["beam"]
    input_files = list(file_to_iov.keys())

    expert_config = kwargs.get("expert_config")
    snrThreshold = expert_config["snrThreshold"]
    computeAverageOccupancyPerChip = expert_config["computeAverageOccupancyPerChip"]
    relativeOccupancyThreshold = expert_config["relativeOccupancyThreshold"]
    absoluteOccupancyThreshold = expert_config["absoluteOccupancyThreshold"]
    skipHLTRejectedEvents = expert_config["skipHLTRejectedEvents"]

    avgOcc = "avgOccPerSensor"
    if (computeAverageOccupancyPerChip):
        avgOcc = "avgOccPerChip"

    exps = [i.exp_low for i in file_to_iov.values()]
    runs = sorted([i.run_low for i in file_to_iov.values()])

    firstRun = runs[0]
    lastRun = runs[-1]
    expNum = exps[0]

    if not len(input_files):
        print("No good input files found! Check that the input files have entries != 0!")
        sys.exit(1)

    uniqueID = f"SVDHotStripsCalibrations_{now.isoformat()}_INFO:"\
               f"_ZS{snrThreshold}_{avgOcc}_relOccThr={relativeOccupancyThreshold}"\
               f"_absOccThr={absoluteOccupancyThreshold}_Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID:\n{uniqueID}")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, requested_iov.exp_high, -1)

    pre_collector_path = create_pre_collector_path(snrThreshold=snrThreshold)

    coll = create_collector(name="SVDOccupancyCalibrationsCollector",
                            svdShaperDigits="SVDShaperDigitsZS",
                            skipHLTRejectedEvents=skipHLTRejectedEvents)

    algo = create_algorithm(uniqueID,
                            computeAverageOccupancyPerChip,
                            relativeOccupancyThreshold,
                            absoluteOccupancyThreshold)

    calibration = Calibration("SVDHotStripsCalibrations",
                              collector=coll,
                              algorithms=[algo],
                              input_files=input_files,
                              pre_collector_path=pre_collector_path)

    calibration.strategies = strategies.SequentialRunByRun

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    return [calibration]
