#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates several events and
# does a simple dE/dx measurement with path length
# correction. The results are stored in a ROOT file.
#
# Usage: basf2 dedxPID_evtgen.py
#
# Input: None
# Output: dedxPID_evtgen.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import DedxDatabaseImporter
import glob

main = create_path()

# ,"localdb/dbstore_wiregains.root_rev_1.root")
use_local_database("calibration_results/CDCDedxWireGainCalibration/outputdb/database.txt")

main.add_module('RootInput', inputFileName='../B2Electrons.root')

main.add_module('DedxCorrection')

main.add_module('RootOutput', outputFileName='NewB2Electrons.root')

process(main)
print(statistics)
