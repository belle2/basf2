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

# Usage: basf2 -i inputFile run_eclGammaGammaE_collector.py
# option: basf2 -i inputFile run_eclGammaGammaE_collector.py OutputFile.root

# Run just the collector part of the eclGammaGammaE calibration, which calibrates the single crystal
# energy response using gamma gamma events.
# Input file should be a gamma gamma skim (or MC) and include ECLDigits, ECLCalDigits, ECLClusters,
# and tracks (to veto Bhabhas).
# Output histograms are written to specified output file. Default is eclGammaGammaECollectorOutput.root
# run_eclGammaGammaE_algorithm.py is then used to perform calibration using these histograms, or to simply copy
# them to an output file.

import os
import sys
from basf2 import *
from ROOT import Belle2


main = create_path()
DR2 = '/ghi/fs01/belle2/bdata/users/karim/MC/DR2/release-01-00-00/gg/mdst/gg_0.root'
main.add_module('RootInput', inputFileNames=[DR2])

narg = len(sys.argv)
outputName = "eclGammaGammaECollectorOutput.root"
if(narg >= 2):
    outputName = sys.argv[1]
main.add_module("HistoManager", histoFileName=outputName)

eclGammaGammaE = register_module('eclGammaGammaECollector')
eclGammaGammaE.param('thetaLabMinDeg', 0.)
eclGammaGammaE.param('thetaLabMaxDeg', 180.)
eclGammaGammaE.param('minPairMass', 9.)
eclGammaGammaE.param('mindPhi', 179.)
eclGammaGammaE.param('maxTime', 1.)
# Can fill histograms with MC eclCalDigits to calculate true deposited energy
eclGammaGammaE.param('measureTrueEnergy', False)
eclGammaGammaE.param('requireL1', True)
main.add_module(eclGammaGammaE)

main.add_module('Progress')

set_log_level(LogLevel.INFO)

# It is possible to force the job to use the specified global tag.
# Default localdb is the subdirectory of current working directory, but can be overwritten
reset_database()
use_database_chain()
use_central_database("development")
use_local_database("localdb/database.txt")

process(main)

print(statistics)
