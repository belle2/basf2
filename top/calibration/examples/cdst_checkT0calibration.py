#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Check T0 calibration with Bhabha (or dimuon) events. Results in a root file.
#
# usage: basf2 cdst_checkT0calibration.py -i <cdst_file.root>
# ---------------------------------------------------------------------------------------

import basf2 as b2

# Global tag
b2.use_central_database('data_reprocessing_proc7')

# Create path
main = b2.create_path()

# Input: cDST file(s) of Bhabha skim, use -i option
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Channel masking
main.add_module('TOPChannelMasker')

# Calibration checker: for dimuon sample replace 'bhabha' with 'dimuon'
calibrator = b2.register_module('TOPChannelT0Calibrator')
calibrator.param('sample', 'bhabha')
calibrator.param('outputFileName', 'checkT0cal_r*.root')
main.add_module(calibrator)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
