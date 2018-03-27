#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Display of waveforms from production firmware
# Usage: basf2 showProdWaveforms.py -i <file_name.sroot>
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH1F, TCanvas, TGraph
from array import array


class WFDisplay(Module):

    '''
    Simple event display of waveforms with feature extraction points
    '''

    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)

    #: output file name
    pdfFile = 'waveforms'

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''
        self.c1.Show()

    def wait(self):
        ''' wait for user respond '''

        try:
            q = 0
            Q = 0
            p = 1
            P = 1
            abc = eval(input('Type <CR> to continue, P to print or Q to quit '))
            if abc == 1:
                return False
            else:
                evtMetaData = Belle2.PyStoreObj('EventMetaData')
                evtMetaData.obj().setEndOfData()
                return True
        except BaseException:
            return False

    def event(self):
        '''
        Event processor: fill the histograms, display them,
        wait for user respond.
        '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        event = evtMetaData.obj().getEvent()
        run = evtMetaData.obj().getRun()

        waveforms = Belle2.PyStoreArray('TOPProductionWaveforms')

        k = 0
        nHits = 0
        fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
        #: output file name
        self.pdfFile = fname
        for i, waveform in enumerate(waveforms):
            print("event %s waveform %s:" % (event, i))
            scrod = waveform.getScrodID()
            carrier = waveform.getCarrier()
            asic = waveform.getASIC()
            channel = waveform.getChannel()
            window = waveform.getWindow()

            graphTitle = "event %s wf no %s: scrod %s carrier %s asic %s chan %s window %s" % (
                event, i, scrod, carrier, asic, channel, window)

            sampleValues = waveform.getWaveform()
            sampleNumbers = waveform.getSampleNumbers()
            numSamples = waveform.getSize()

            sampleValuesArray = array('d')
            sampleNumbersArray = array('d')
            for i, (sampleValue, sampleNumber) in enumerate(zip(sampleValues, sampleNumbers)):
                sampleValuesArray.append(sampleValue)
                sampleNumbersArray.append(sampleNumber)
                # print ("%s/%s: %s %s" % (i,numSamples,sampleValue, sampleNumber))

            graph = TGraph(numSamples, sampleNumbersArray, sampleValuesArray)
            graph.SetTitle(graphTitle)

            self.c1.Clear()

            graph.Draw('ALP')
            # hist.Draw()

            self.c1.Update()
            r = self.wait()
            if r:
                break


set_log_level(LogLevel.INFO)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP (needed only in PocketDAQ)
rawconverter = register_module('Convert2RawDet')
main.add_module(rawconverter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking
unpack = register_module('TOPUnpacker')
# unpack.param('swapBytes', True)
# unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Add multiple hits from waveforms
featureExtractor = register_module('TOPWaveformFeatureExtractor')
# main.add_module(featureExtractor)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
# main.add_module(converter)

# Display waveforms
main.add_module(WFDisplay())

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
