#!/usr/bin/env python3
# -*- coding: utf-8 -*-
########################################################
# This steering file creates ECL DQM histograms from
# input SeqRoot file
#
# The user should provide input and output filenames and
# control EclDQM module parameters
#
# Example steering file - 2018 Belle II Collaboration
#
# Dmitry Matvienko (d.v.matvienko@inp.nsk.su)
#
########################################################

import basf2 as b2

b2.set_log_level(b2.LogLevel.ERROR)

# Register necessary modules
input = b2.register_module('SeqRootInput')

unpacker = b2.register_module("ECLUnpacker")
unpacker.param("storeTrigTime", True)

calib = b2.register_module('ECLDigitCalibrator')

histo = b2.register_module('HistoManager')

trigger_skim_module = b2.register_module("TriggerSkim")
trigger_skim_module.param('triggerLines', ["software_trigger_cut&all&total_result"])
trigger_skim_module.if_value("==0", Path(), AfterConditionPath.END)

ecldqm = b2.register_module('ECLDQM')
ecldqm.param("histogramDirectoryName", "ECL")
ecldqm.param("EnergyUpperThr", 1.5)
ecldqm.param("PedestalMeanUpperThr", 7000)
ecldqm.param("PedestalMeanLowerThr", -1000)
ecldqm.param("PedestalRmsUpperThr", 100.)

# Create paths
main = b2.create_path()
main.add_module(input)
main.add_module(unpacker)
main.add_module(calib)
main.add_module(histo)
main.add_module(trigger_skim_module)
main.add_module(ecldqm)

# Process all events
b2.process(main)
print(b2.statistics)
