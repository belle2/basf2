#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2
from ROOT import TH2F, TFile, gDirectory

#
# ------------------------------------------------------------------------
# Simple module to study the width and amplitude of TOPDigits
#
# usage: basf2 checkHitHeightAndWidth.py -i <raw_data.sroot>
# ------------------------------------------------------------------------


class Histogrammer(b2.Module):

    ''' A module to histogram width and amplitude of TOPDigits '''

    def initialize(self):
        ''' Open output file and book histograms '''

        #: output file
        self.tfile = TFile.Open('hitHeightAndWidth.root', 'recreate')
        dirs = [gDirectory.mkdir('slot_' + '{:0=2d}'.format(slot)) for slot in range(1, 17)]

        #: Width VS amplitude plot in each slot
        self.hist = []
        #: Width as function of the sample number in each channel
        self.histSampling = []

        for slot in range(1, 17):
            dirs[slot - 1].cd()
            h = TH2F('WidthVSAmplitude', 'Slot ' + str(slot) +
                     ';amplitude [ADC counts]; width [ns]', 2000, 0, 2000, 200, 0, 10)
            self.hist.append(h)
            hh = [TH2F('WidthVSample_ch' + '{:0=3d}'.format(ch), 'Slot ' + str(slot) + ', channel ' + str(ch) +
                       ';sample; width [ns]', 256, 0, 256, 200, 0, 10) for ch in range(512)]
            self.histSampling.append(hh)

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            if digit.getHitQuality() == 0:
                continue
            slotID = digit.getModuleID()
            hwchan = digit.getChannel()
            self.hist[slotID - 1].Fill(digit.getPulseHeight(), digit.getPulseWidth())
            self.histSampling[slotID - 1][hwchan].Fill(digit.getModulo256Sample(), digit.getPulseWidth())

    def terminate(self):
        ''' Write histograms to file '''

        self.tfile.Write()
        self.tfile.Close()


# Suppress messages and warnings during processing
b2.set_log_level(b2.LogLevel.ERROR)

# Define a global tag
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
# roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
main.add_module(converter)

# Histogrammer
histogramModule = Histogrammer()
main.add_module(histogramModule)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
