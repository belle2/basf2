#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH2F, TFile

# ------------------------------------------------------------------------
# example of making histograms with the width and amplidute of all the topdigits
# needs as input the file produced by top/tools/simLaserCalibration.py
# ------------------------------------------------------------------------


class Histogrammer(Module):

    ''' A module to histogram width and amplitude of calibration pulses'''

    #: 2D histograms
    hist = [TH2F('WidthVSAmplitude_Slot_' + str(k + 1), 'With VS amplidute of the Digits in slot ' + str(k + 1),
                 2000, 0., 2000, 80, 0., 20) for k in range(15)]

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            quality = digit.getHitQuality()
            if quality == 4:
                slotID = digit.getModuleID()
                hwchan = digit.getChannel()
                self.hist[slotID - 1].Fill(digit.getPulseHeight(), digit.getPulseWidth())

    def terminate(self):
        ''' Write histograms to file '''

        tfile = TFile('calpulseCheck.root', 'recreate')
        for k in range(9):
            self.hist[k].Write()
        self.propTime.Write()
        tfile.Close()


argvs = sys.argv
if len(argvs) is not 5:
    print('usage: basf2', argvs[0],
          '-i <file_sroot> (pocket|local) <slot> <channel> <output_dir>')
    sys.exit()

datatype = argvs[1]      # data type (pocket, local)
slot = int(argvs[2])     # slot number (1-16)
channel = int(argvs[3])  # calibration channel (0-7)
outdir = argvs[4]        # output directory path

print('data type:', datatype, ' slot:', slot, ' calibration channel:', channel,
      ' output to:', outdir)

# Suppress messages and warnings during processing
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if datatype == 'pocket':
    print('pocket DAQ data assumed')
    converter = register_module('Convert2RawDet')
    main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking
unpack = register_module('TOPUnpacker')
unpack.param('swapBytes', True)
unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
featureExtractor = register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('calibrationChannel', channel)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 8.0)  # in [ns]
converter.param('lookBackWindows', 29)  # in number of windows
main.add_module(converter)

# Histogrammer
main.add_module(Histogrammer())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
print(statistics(statistics.TERM))
