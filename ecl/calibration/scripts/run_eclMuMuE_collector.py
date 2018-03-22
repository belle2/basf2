#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2017  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# Usage: basf2 run_eclMuMuE_collector.py
# option: basf2 run_eclMuMuE_collector.py OutputFileName.root

# Run just the collector part of the eclMuMuE calibration, which calibrates the single crystal
# energy response using muon pairs.
# Input file should be a muon pair skim (or MC) and include ECLDigits and tracks.
# Also requires ECLCalDigits if code is being used to find expected deposited energies.
# Output histograms are written to specified output file.
# run_eclMuMuE_algorithm.py is then used to perform calibration using these histograms, or to simply copy
# them to an output file.

import os
import sys
from basf2 import *
from ROOT import Belle2


main = create_path()
DR2 = '/ghi/fs01/belle2/bdata/users/karim/MC/DR2/release-00-09-02/mdst_calib/*.root'
main.add_module('RootInput', inputFileNames=[DR2])

narg = len(sys.argv)
outputName = "eclMuMuECollectorOutput.root"
if(narg >= 2):
    outputName = sys.argv[1]
main.add_module("HistoManager", histoFileName=outputName)

# Genfit and track extrapolation
gearbox = register_module('Gearbox')
main.add_module(gearbox)
geometry = register_module('Geometry')
main.add_module(geometry)
main.add_module("SetupGenfitExtrapolation")
ext = register_module('Ext')

# extrapolate using muon hypothesis only
pdgcodes = [13]
ext.param('pdgCodes', pdgcodes)

main.add_module(ext)

eclMuMuE = register_module('eclMuMuECollector')
eclMuMuE.param('minPairMass', 9.0)
eclMuMuE.param('minTrackLength', 30.)
eclMuMuE.param('MaxNeighbourE', 0.010)
eclMuMuE.param('thetaLabMinDeg', 17.)
eclMuMuE.param('thetaLabMaxDeg', 150.)
# Can fill histograms with MC eclCalDigits to calculate true deposited energy
eclMuMuE.param('measureTrueEnergy', False)
eclMuMuE.param('requireL1', True)
main.add_module(eclMuMuE)

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
