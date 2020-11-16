#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty hearty@physics.ubc.ca
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# Usage: basf2 -i inputFile run_ee5x5_collector_mc.py
# option: basf2 -i inputFile run_ee5x5_collector_mc.py OutputFile.root

# Run the collector part of the eclee5x5 calibration, which calibrates the single crystal
# energy response using Bhabha events.

# This script sets parameters appropriate for mc. Use run_ee5x5_collector_data.py for data

# Input file should be cdst file, including ECLDigits, ECLCalDigits, and clusters
# Output histograms are written to specified output file.

# run_ee5x5_algorithm_XXX.py is then used to find payloads or output merged histograms.

import sys
import basf2 as b2


main = b2.create_path()
DR2 = '/ghi/fs01/belle2/bdata/users/karim/MC/DR2/release-02-01-00/eegamma/cdst/eegamma_1.root'
main.add_module('RootInput', inputFileNames=[DR2])

narg = len(sys.argv)
outputName = "ee5x5CollectorOutput_mc.root"
if(narg >= 2):
    outputName = sys.argv[1]
main.add_module("HistoManager", histoFileName=outputName)

eclee5x5 = b2.register_module('eclee5x5Collector')
eclee5x5.param('thetaLabMinDeg', 17.)
eclee5x5.param('thetaLabMaxDeg', 150.)
eclee5x5.param('minE0', 0.45)
eclee5x5.param('minE1', 0.40)
eclee5x5.param('maxdThetaSum', 2.)
eclee5x5.param('dPhiScale', 1.)
eclee5x5.param('maxTime', 10.)
# Can fill histograms with eclCalDigits to calculate deposited energy using MC
eclee5x5.param('useCalDigits', True)
eclee5x5.param('requireL1', False)
main.add_module(eclee5x5)

main.add_module('Progress')

b2.set_log_level(b2.LogLevel.INFO)

# Force the job to use the global tag for offline calibration.
# Default localdb is the subdirectory of current working directory, but can be overwritten
b2.reset_database()
b2.use_database_chain()
b2.use_central_database("Calibration_Offline_Development")
b2.use_local_database("localdb/database.txt")

b2.process(main)

print(b2.statistics)
