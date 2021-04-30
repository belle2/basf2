#!/usr/bin/env python3
########################################################################
#
# Test script for the SVDDQMDose module.
#
# Author: Ludovico Massaccesi
#
# Usage:
#   basf2 SVDDQMDoseModuleExample.py -- file.root [file2.root ...] \
#   [--out-file output_file.root]
#
# The rootfiles MUST contain RAW DATA.
#
# 2020 Belle II Collaboration
#
########################################################################
"""Uses the SVDDQMDose module and makes a rootfile with the histos."""
import argparse
import os
import basf2 as b2
from basf2 import conditions
from rawdata import add_unpackers
from svd import add_svd_reconstruction
from svd.executionTime_utils import SVDExtraEventStatisticsModule
from svd.dqm_utils import add_svd_dqm_dose


def prepend_to_filename(file_path, prefix):
    dn, bn = os.path.dirname(file_path), os.path.basename(file_path)
    return os.path.join(dn, f"{prefix}{bn}")


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("files", metavar="FILE", nargs="+",
                    help="The input rootfile(s) with the RAW data.")
parser.add_argument("-o", "--out-file", default="SVDDQM.root",
                    help='The output rootfile. Default "SVDDQM.root".')
parser.add_argument("--no-trg-filter", action="store_true",
                    help="Take all events instead of TTYP_POIS only.")
parser.add_argument("--exec-time", action="store_true",
                    help="Also record execution time statistics.")
args = parser.parse_args()

conditions.override_globaltags()
conditions.globaltags = ['svd_onlySVDinGeoConfiguration', 'online',
                         'Reco_master_patch_rel5']  # For HardwareClockSettings

main = b2.create_path()

# Read input (RAW data)
main.add_module("RootInput", inputFileNames=args.files)

# HistoManager and output
main.add_module("HistoManager", histoFileName=args.out_file)

# Necessary modules
main.add_module('Gearbox')
main.add_module('Geometry')
if args.no_trg_filter:
    add_unpackers(main, components=['SVD'])
else:
    add_unpackers(main, components=['SVD', 'TRG'])
main.add_module(
    "SVDZeroSuppressionEmulator", SNthreshold=5,
    ShaperDigits='SVDShaperDigits', ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)
if args.exec_time:
    add_svd_reconstruction(main)  # Required for the statistics

# SVDDQMDose module (Poisson trigger only)
params = {'trgTypes': []} if args.no_trg_filter else {}
add_svd_dqm_dose(main, "SVDShaperDigitsZS5", **params)

# SVDDQMInjection for execution time comparison
main.add_module('SVDDQMInjection', ShaperDigits='SVDShaperDigitsZS5')

# Execution time statistics
if args.exec_time:
    main.add_module(SVDExtraEventStatisticsModule(
        prepend_to_filename(args.out_file, "stats_")))

# Necessary for impatient humans :)
main.add_module("ProgressBar")

# b2.print_path(main)
b2.process(main)
print(b2.statistics)
