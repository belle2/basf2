#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Check T0 calibration with Bhabha (or dimuon) events. Results in a root file.
#
# usage: basf2 cdst_checkT0calibration.py -i <cdst_file.root>
# ---------------------------------------------------------------------------------------

from basf2 import *
import glob
import sys

# Global tag
use_central_database('data_reprocessing_proc7')  # use the correct global tag

# Create path
main = create_path()

# Input: cDST file(s) of Bhabha skim, use -i option
roinput = register_module('RootInput')
main.add_module(roinput)

# Geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
geometry.param('useDB', False)
main.add_module(geometry)

# Channel masking
main.add_module('TOPChannelMasker')

# Calibration checker: results saved in calibrationT0_r<runNo>.root
calibrator = register_module('TOPChannelT0Calibrator')
calibrator.param('sample', 'bhabha')
main.add_module(calibrator)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
