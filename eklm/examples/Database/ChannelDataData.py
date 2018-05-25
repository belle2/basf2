#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

from basf2 import *
import ROOT
import sys
from ROOT.Belle2 import EKLMDatabaseImporter, EKLMChannelData

if (len(sys.argv) != 3):
    print('Usage: basf2 ChannelDataData.py calibration_data threshold_shift')
    exit(1)

calibration_data = sys.argv[1]
threshold_shift = sys.argv[2]

set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')

# Gearbox
gearbox = register_module('Gearbox')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

process(main)

# Create default data
channel_data = EKLMChannelData()
channel_data.setActive(False)
channel_data.setPedestal(0)
channel_data.setPhotoelectronAmplitude(0)
channel_data.setThreshold(0)
channel_data.setAdjustmentVoltage(0)
channel_data.setLookbackWindow(0)

# Load default data
dbImporter = EKLMDatabaseImporter()
dbImporter.loadChannelData(channel_data)

# Load channel data
channel_data_inactive = EKLMChannelData(channel_data)
channel_data_inactive.setActive(False)
dbImporter.loadChannelDataCalibration(calibration_data, int(threshold_shift))

# Import data
dbImporter.importChannelData()
