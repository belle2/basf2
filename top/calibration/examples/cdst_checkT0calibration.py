#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Check T0 calibration with Bhabha (or dimuon) events. Results in a root file.
#
# usage: basf2 cdst_checkT0calibration.py -i <cdst_file.root>
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys

# Global tag
use_central_database('data_reprocessing_proc7')

# Create path
main = create_path()

# Input: cDST file(s) of Bhabha skim, use -i option
roinput = register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Channel masking
main.add_module('TOPChannelMasker')

# Calibration checker: for dimuon sample replace 'bhabha' with 'dimuon'
calibrator = register_module('TOPChannelT0Calibrator')
calibrator.param('sample', 'bhabha')
calibrator.param('outputFileName', 'checkT0cal_r*.root')
main.add_module(calibrator)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
