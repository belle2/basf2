#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
from basf2 import create_path, process, LogLevel, set_log_level, set_random_seed, use_central_database
from datastoreprinter import DataStorePrinter, PrintObjectsModule

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
    DataStorePrinter("BKLMDigit", ["getUniqueChannelID", "getTime", "getEDep", "getCharge", "getTime"]),
    DataStorePrinter("EKLMDigit", ["getUniqueChannelID", "getCTime", "getCharge", "getPlane", "getStrip"]),
]


def unpack_and_print_files(filenames):
    """
    process a given files and print its unpacked raw contents
    Needs to all happen in one process call, otherwise the Geomtery would
    be loaded multiple times, which results in an error
    """
    print("Testing raw input files {}".format([os.path.basename(f) for f in filenames]))

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


def test_raw(phase_name, global_tag):
    """
    Runs the whole Raw unpacking testing scheme for one global tag for phase 2 or phase 3 events
    """

    set_log_level(LogLevel.ERROR)
    set_random_seed(1)
    use_central_database(global_tag)

    if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
        print("TEST SKIPPED: rawdata_compatibility because BELLE2_VALIDATION_DATA_DIR environment variable not set.",
              file=sys.stderr)
        sys.exit(1)

    rawdata_path = os.path.join(os.environ['BELLE2_VALIDATION_DATA_DIR'], "rawdata", phase_name)
    unpack_and_print_files(glob.glob(rawdata_path + "/*.root"))
