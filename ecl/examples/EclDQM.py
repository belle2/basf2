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
import os
import sys
from subprocess import call
from basf2 import *

set_log_level(LogLevel.ERROR)

# Register necessary modules
input = register_module('SeqRootInput')

unpacker = register_module("ECLUnpacker")
unpacker.param("storeTrigTime", True)

calib = register_module('ECLDigitCalibrator')

histo = register_module('HistoManager')

ecldqm = register_module('ECLDQM')
ecldqm.param("histogramDirectoryName", "ECL")
ecldqm.param("EnergyUpperThr", 1.5)
ecldqm.param("PedestalMeanUpperThr", 7000)
ecldqm.param("PedestalMeanLowerThr", -1000)
ecldqm.param("PedestalRmsUpperThr", 100.)

# Create paths
main = create_path()
main.add_module(input)
main.add_module(unpacker)
main.add_module(calib)
main.add_module(histo)
main.add_module(ecldqm)

# Process all events
process(main)
print(statistics)
