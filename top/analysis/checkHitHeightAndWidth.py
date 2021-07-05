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
import sys
import glob
from ROOT import Belle2
from ROOT import TH2F, TFile

#
# ------------------------------------------------------------------------
# Simple module to study the width and amplitude of all the topdigits
#
# usage: basf2 checkHitHeightAndWidth.py run_number output_name.root
#        the run number accepts wildcards
# ------------------------------------------------------------------------


class Histogrammer(b2.Module):

    ''' A module to histogram width and amplitude of calibration pulses'''

    #: Width VS amplitude plot in each slot
    hist = [TH2F('WidthVSAmplitude_Slot_' + str(k + 1), 'With VS amplidute of the Digits in slot ' + str(k + 1),
                 2000, 0., 2000, 80, 0., 20) for k in range(16)]

    #: Width as function of the sample number in each channel
    histSampling = [[TH2F('WidthVSample_Slot_' + str(k + 1) + '_Channel_' + str(j),
                          'With VS amplidute of the Digits in slot ' + str(k + 1) + ' channel ' + str(j),
                          256,
                          0.,
                          256,
                          100,
                          0.,
                          20) for k in range(16)] for j in range(512)]
    #: Default output name
    outname = 'calpulseCheck.root'

    def setOutputName(self, outputname):
        ''' Sets the output file name '''

        #: output name
        self.outname = outputname

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            # quality = digit.getHitQuality()
            slotID = digit.getModuleID()
            hwchan = digit.getChannel()
            self.hist[slotID - 1].Fill(digit.getPulseHeight(), digit.getPulseWidth())
            self.histSampling[hwchan][slotID - 1].Fill(digit.getModulo256Sample(), digit.getPulseWidth())

    def terminate(self):
        ''' Write histograms to file '''

        tfile = TFile(self.outname, 'recreate')
        for k in range(16):
            self.hist[k].GetXaxis().SetTitle("TOPDigit amplitude [ADC counts]")
            self.hist[k].GetYaxis().SetTitle("TOPDigit width [ns]")
            self.hist[k].Write()
        for k in range(16):
            for j in range(512):
                self.histSampling[j][k].GetXaxis().SetTitle("TOPDigit sample%256")
                self.histSampling[j][k].GetYaxis().SetTitle("TOPDigit width [ns]")
                self.histSampling[j][k].Write()

        tfile.Close()


argvs = sys.argv

print('usage: basf2', argvs[0], 'runNumber outfileName')

runnumber = argvs[1]   # run number. Wildcards should work
outfile = argvs[2]     # output name


files = glob.glob('/ghi/fs01/belle2/bdata/Data/sRaw/e0001/r0' + str(runnumber) + '/sub00/*.sroot')

# Suppress messages and warnings during processing
b2.set_log_level(b2.LogLevel.ERROR)

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
roinput.param('inputFileNames', files)
main.add_module(roinput)

#
# conversion from RawCOPPER or RawDataBlock to RawDetector objects
# Pocket DAQ data not supported yet
#
# if datatype == 'pocket':
#    print('pocket DAQ data assumed')
#    converter = register_module('Convert2RawDet')
#    main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
featureExtractor = b2.register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
# converter.param('calibrationChannel', channel)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 8.0)  # in [ns]
converter.param('lookBackWindows', 29)  # in number of windows
main.add_module(converter)

# Histogrammer
histogramModule = Histogrammer()
histogramModule.setOutputName(outfile)
main.add_module(histogramModule)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
print(b2.statistics(b2.statistics.TERM))
