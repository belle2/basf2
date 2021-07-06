#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test backwards compatibility for RAW Objects and Unpackers.

The ability to unpack all recorded Belle II events also with future release is
vital to be able to reconstruct the full Belle II dataset during the lifetime of the experiment
and after that.
This is the basis of unit tests which execute the unpackers and checks for the properties of the
unpacked sub-detector StoreArrays.
"""

import sys
import os
import glob
from ROOT import Belle2
from basf2 import create_path, process, LogLevel, set_log_level, set_random_seed, conditions
from b2test_utils import require_file
from b2test_utils.datastoreprinter import DataStorePrinter, PrintObjectsModule

from rawdata import add_unpackers


# Now we define a list of all the unpacker output we want to print out and all
# the members we want to check
unpacked_dataobjects = [
    DataStorePrinter("PXDRawHit", ["getRow", "getColumn", "getCharge"]),
    DataStorePrinter("SVDShaperDigit", ["getCellID", "getFADCTime", "toString"]),
    DataStorePrinter("CDCHit", ["getID", "getStatus", "getTDCCount", "getADCCount"]),
    DataStorePrinter("ECLDigit", ["getCellId", "getAmp", "getTimeFit"]),
    DataStorePrinter("TOPDigit", ["getModuleID", "getPixelID", "getPixelRow", "getPixelCol", "getRawTime",
                                  "getPulseHeight", "getPulseWidth"]),
    DataStorePrinter("ARICHDigit", ["getModuleID", "getChannelID", "getBitmap"]),
    DataStorePrinter("BKLMDigit", ["getUniqueChannelID", "getTime", "getEnergyDeposit", "getCharge", "getTime"]),
    DataStorePrinter("EKLMDigit", ["getUniqueChannelID", "getCTime", "getCharge", "getPlane", "getStrip"]),
    DataStorePrinter("CDCTriggerSegmentHit", ["getID"]),
    DataStorePrinter("CDCTrigger2DFinderTrack", ["getTime", "getPt"]),
    DataStorePrinter("CDCTriggerNeuroTrack", ["getTime", "getPt"]),
]


def unpack_and_print_files(filenames):
    """
    process a given files and print its unpacked raw contents
    Needs to all happen in one process call, otherwise the Geomtery would
    be loaded multiple times, which results in an error
    """

    if len(filenames) == 0:
        print("TEST SKIPPED: No input files for test",
              file=sys.stderr)
        sys.exit(1)

    raw_files = [Belle2.FileSystem.findFile(f) for f in filenames]
    main = create_path()
    main.add_module("RootInput", inputFileNames=raw_files, logLevel=LogLevel.WARNING)

    main.add_module("EventInfoPrinter")

    main.add_module("Gearbox")
    main.add_module("Geometry", useDB=True)
    add_unpackers(main)
    main.add_module(PrintObjectsModule(unpacked_dataobjects))
    # only 5 events for now, otherwise the text file for comparison will be to big
    # for git
    process(main, 5)


def test_raw(phase_name, postfix, global_tag):
    """
    Runs the whole Raw unpacking testing scheme for one global tag for phase 2 or phase 3 events
    """

    set_log_level(LogLevel.ERROR)
    set_random_seed(1)
    # only override the default global tag if a specific GT was provided for this test
    if global_tag:
        conditions.override_globaltags([global_tag])
    else:
        # otherwise use current default globaltag
        conditions.disable_globaltag_replay()

    rawdata_path = require_file(os.path.join('rawdata', phase_name), "validation")
    unpack_and_print_files(glob.glob(os.path.join(rawdata_path, f"{postfix}*.root")))
