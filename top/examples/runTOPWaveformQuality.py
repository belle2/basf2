#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Display of waveforms with feature extraction points superimposed
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False  # noqa
from ROOT import gROOT
gROOT.SetBatch()  # noqa

import basf2 as b2

b2.set_log_level(b2.LogLevel.INFO)

# Database
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP
converter = b2.register_module('Convert2RawDet')
main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# TOP's data quality module
histomanager = b2.register_module("HistoManager")
main.add_module(histomanager)
wfqp = b2.register_module('TOPWaveformQualityPlotter')
main.add_module(wfqp)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
