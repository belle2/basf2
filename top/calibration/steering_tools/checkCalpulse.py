#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------------------------------------------
# Makes histograms from TOPDigits to decide visually about calpulse selection settings.
# Input: pulser or laser raw data in root format (local run)
# Output: checkCalpulse.root
#
# usage: basf2 checkCalpulse.py -i <local_run.root> [cal_channel] [time_cut_lo]  [time_cut_hi] [localDB]
#        cal_channel (optional) calibration channel number (0 - 7), if given TOPDigits in other ASIC channels will be ignored
#        time_cut_lo (optional) cut on hit time, if given TOPDigits with time < time_cut_lo will be ignored
#        time_cut_hi (optional) cut on hit time, if given TOPDigits with time > time_cut_hi will be ignored
#        localDB     (optional) local database or global tag, if given the time base calibration will be used
# ---------------------------------------------------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
from ROOT import TFile, TH1F, TH2F
import sys

look_back_windows = 30  # make sure this is the correct setting for your local run!

cal_channel = None
if len(sys.argv) > 1:
    cal_channel = int(sys.argv[1])
time_cut_lo = None
if len(sys.argv) > 2:
    time_cut_lo = float(sys.argv[2])
time_cut_hi = None
if len(sys.argv) > 3:
    time_cut_hi = float(sys.argv[3])
use_sample_times = False
use_asic_shifts = False
if len(sys.argv) > 4:
    use_sample_times = True
    use_asic_shifts = True
    tag = sys.argv[4]
    if '.txt' in tag:
        b2.conditions.append_testing_payloads(tag)
    else:
        b2.conditions.expert_settings(usable_globaltag_states={'PUBLISHED', 'RUNNING', 'TESTING', 'VALIDATED', 'OPEN'})
        b2.conditions.prepend_globaltag(tag)


class Histogrammer(b2.Module):
    ''' Makes histograms from TOPDigits '''

    def initialize(self):
        ''' initialize: open root file, book histograms '''

        #: file object
        self.tfile = TFile.Open('checkCalpulse.root', 'recreate')
        #: histogram of pulse height vs. pulse width
        self.height_vs_width = TH2F("height_vs_width", "Pulse height vs. width; pulse width [ns]; pulse height [ADC counts]",
                                    100, 0, 10, 200, 0, 2000)
        #: histogram of pulse height vs. sample number
        self.height_vs_sample = TH2F("height_vs_sample", "Pulse height vs. sample; sample number; pulse height [ADC counts]",
                                     256, 0, 256, 200, 0, 2000)
        #: histogram of pulse width vs. sample number
        self.width_vs_sample = TH2F("width_vs_sample", "Pulse width vs. sample; sample number; pulse width [ns]",
                                    256, 0, 256, 100, 0, 10)
        #: histogram of time distribution
        self.time = TH1F('time', 'Time distibution; time [ns]', 400, -100, 300)
        #: histogram of asic channel occupancy
        self.asic_channels = TH1F('asic_channels', 'ASIC channel occupancy; ASIC channel number', 8, 0, 8)

    def event(self):
        ''' event processing: fill histograms '''

        for digit in Belle2.PyStoreArray('TOPDigits'):
            if digit.getHitQuality() == 0:
                continue
            if cal_channel is not None and digit.getASICChannel() != cal_channel:
                continue
            if time_cut_lo is not None and digit.getTime() < time_cut_lo:
                continue
            if time_cut_hi is not None and digit.getTime() > time_cut_hi:
                continue
            self.height_vs_width.Fill(digit.getPulseWidth(), digit.getPulseHeight())
            self.height_vs_sample.Fill(digit.getModulo256Sample(), digit.getPulseHeight())
            if digit.getPulseHeight() > 100:
                self.width_vs_sample.Fill(digit.getModulo256Sample(), digit.getPulseWidth())
            self.time.Fill(digit.getTime())
            self.asic_channels.Fill(digit.getASICChannel())

    def terminate(self):
        ''' terminate: write histograms and close the file '''

        h = self.height_vs_width.ProjectionX('width')
        h.SetTitle('Pulse width')
        h = self.height_vs_width.ProjectionY('height')
        h.SetTitle('Pulse height')
        self.tfile.Write()
        self.tfile.Close()


main = b2.create_path()
main.add_module('RootInput')
main.add_module('TOPGeometryParInitializer')
main.add_module('TOPUnpacker')
main.add_module('TOPRawDigitConverter',
                useSampleTimeCalibration=use_sample_times,
                useAsicShiftCalibration=use_asic_shifts,
                useChannelT0Calibration=False,
                useModuleT0Calibration=False,
                useCommonT0Calibration=False,
                minPulseWidth=0.5,
                lookBackWindows=look_back_windows)

main.add_module(Histogrammer())
main.add_module('Progress')

# Process events
b2.process(main)
