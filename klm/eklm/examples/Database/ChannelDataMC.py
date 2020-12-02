#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM channel data payloads.

import basf2
from ROOT.Belle2 import EKLMDatabaseImporter, EKLMChannelData

basf2.set_log_level(basf2.LogLevel.INFO)

# Create default data
channel_data = EKLMChannelData()
channel_data.setActive(True)
channel_data.setPedestal(3700)
channel_data.setPhotoelectronAmplitude(20)
# ADCPedestal - 3.0 * ADCPEAmplitude = 3700 - 3.0 * 20 = 3640
channel_data.setThreshold(3640)
channel_data.setVoltage(72.2)
channel_data.setAdjustmentVoltage(0)
channel_data.setLookbackTime(0)
channel_data.setLookbackWindowWidth(0)

# Load default data
dbImporter = EKLMDatabaseImporter()
dbImporter.loadChannelData(channel_data)

# Load inactive channel
channel_data_inactive = EKLMChannelData(channel_data)
channel_data_inactive.setActive(False)
# dbImporter.setChannelData(1, 1, 1, 1, 40, channel_data_inactive)

# Import data
dbImporter.importChannelData()
