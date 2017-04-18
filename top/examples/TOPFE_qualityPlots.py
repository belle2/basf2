#!/usr/bin/env python
# -*- coding: utf-8 -*-
######################################################
# debug script to dump raw header information
#
# (loosely) based on Root2Binary.py
######################################################

from basf2 import *
from ROOT import Belle2, gROOT
from ROOT import TNtuple, TFile, TH1F, TCanvas, TGraph
import sys
import h5py
import numpy as np
import peakutils
import argparse
parser = argparse.ArgumentParser(description='Extracts information about the FE parameters',
                                 usage='%(prog)s [options]')
parser.add_argument(
    '--plotWaveforms',
    action='store_true',
    help='whether to print out suspicious waveforms')

# args = parser.parse_args()

set_log_level(LogLevel.INFO)
gROOT.SetBatch()
argvs = sys.argv


def wf_display(waveform, run, event, suffix=""):
    '''
    Simple event display of waveforms with feature extraction points
    '''
#: 1D histograms
    hist = TH1F('h', 'wf', 64 * 4, 0.0, 64.0 * 4)
    hist.SetXTitle("sample number")
    hist.SetYTitle("sample value [ADC counts]")
    hist.SetLabelSize(0.06, "XY")
    hist.SetTitleSize(0.07, "XY")
    hist.SetTitleOffset(0.7, "XY")
    #: graphs
    graph = TGraph(10)
    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)
    #: output file name
    fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
    pdfFile = fname
    chan = waveform.getChannel()
    pdfFile = pdfFile + '-' + str(chan)

    nHits = 0
    wf = waveform.getWaveform()
    hist.Reset()
    numSamples = waveform.getSize()
    hist.SetBins(numSamples, 0.0, float(numSamples))
    title = 'chan ' + str(chan) + ' win'
    for window in waveform.getReferenceWindows():
        title += ' ' + str(window)
    hist.SetTitle(title)
    hist.SetStats(False)
    hist.SetLineColor(4)
    if not waveform.areWindowsInOrder():
        hist.SetLineColor(3)
    i = 0
    for sample in wf:
        i = i + 1
        hist.SetBinContent(i, sample)

    rawDigits = waveform.getRelationsWith("TOPRawDigits")
    i = 0
    for raw in rawDigits:
        graph.SetPoint(i, raw.getSampleRise() + 0.5, raw.getValueRise0())
        i += 1
        graph.SetPoint(i, raw.getSampleRise() + 1.5, raw.getValueRise1())
        i += 1
        graph.SetPoint(i, raw.getSamplePeak() + 0.5, raw.getValuePeak())
        i += 1
        graph.SetPoint(i, raw.getSampleFall() + 0.5, raw.getValueFall0())
        i += 1
        graph.SetPoint(i, raw.getSampleFall() + 1.5, raw.getValueFall1())
        i += 1
        if raw.isFEValid():  # works properly only for single rawDigit!
            graph.SetMarkerColor(2)
            if raw.isPedestalJump():
                graph.SetMarkerColor(3)
        else:
            graph.SetMarkerColor(4)
        graph.Set(i)
    c1.Clear()
    title = 'WF event display:' + ' run ' + str(run) + ' event ' \
        + str(event)
    c1.SetTitle(title)
    hist.Draw()
    graph.SetMarkerStyle(24)
    graph.Draw("sameP")
    c1.Update()
    c1.SaveAs(pdfFile + suffix + '.pdf')


class WaveformDumper(Module):
    '''
    Dumps top digit information into hdf5
    '''

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''
        print("WaveformDumper init")
        self.feProps_calPulse = TNtuple(
            "feProps_calPulse",
            "feProps_calPulse",
            "pyPeak0Ht:pyPeak1Ht:fePeakHt:fePeakWd:fePeakIntegral:nPyPeaks")
        self.feProps_signal = TNtuple("feProps_signal", "feProps_signal", "pyPeak0Ht:fePeakHt:fePeakWd:fePeakIntegral:nPyPeaks")
        self.nWaveForms = 0
        self.nWaveFormsOutOfOrder = 0

    def event(self):
        '''
        Event processor: get data and print to screen
        '''
        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        event = evtMetaData.obj().getEvent()
        run = evtMetaData.obj().getRun()
        waveforms = Belle2.PyStoreArray('TOPRawWaveforms')
        for waveform in waveforms:
            self.nWaveForms += 1
            if not waveform.areWindowsInOrder():
                self.nWaveFormsOutOfOrder += 1
            chan = waveform.getChannel()
            wf = np.array(waveform.getWaveform())
            indexes = peakutils.indexes(wf, thres=0.5, min_dist=20)
            rawDigits = waveform.getRelationsWith("TOPRawDigits")
            nPyPeaks = len(indexes)
            if len(rawDigits) != 1:
                print("Warning: #TOPRawDigits is not 1, but", len(rawDigits))
            raw = rawDigits[0]
            fePeakHt = raw.getValuePeak()
            fePeakWd = raw.getSampleFall() - raw.getSampleRise()
            fePeakIntegral = raw.getIntegral()
            # we are sorting the peaks. The FE should always point to the highest one
            idx = sorted(wf[indexes], reverse=True)
            pyPeak0Ht = -1
            if nPyPeaks > 2 and 0 == 1:
                wf_display(waveform, run, event, "manyPeaks")
            elif nPyPeaks > 0:
                pyPeak0Ht = idx[0]
            if pyPeak0Ht != fePeakHt and 0 == 1:
                print(pyPeak0Ht, fePeakHt)
                wf_display(waveform, run, event, "notHighest")
            if chan == 0:
                pyPeak1Ht = -1
                if nPyPeaks > 1:
                    pyPeak1Ht = idx[1]
                self.feProps_calPulse.Fill(pyPeak0Ht, pyPeak1Ht, fePeakHt, fePeakWd, fePeakIntegral, nPyPeaks)
            else:
                self.feProps_signal.Fill(pyPeak0Ht, fePeakHt, fePeakWd, fePeakIntegral, nPyPeaks)

    def terminate(self):
        f = TFile.Open("ntuple.root", "RECREATE")
        f.WriteTObject(self.feProps_calPulse)
        f.WriteTObject(self.feProps_signal)
        f.Close()
        print("# Waveforms:", self.nWaveForms)
        print("out of order:", self.nWaveFormsOutOfOrder)


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
# Create path
main = create_path()
reader = register_module('SeqRootInput')
# file to read
reader.param('inputFileName', argvs[1])
main.add_module(reader)

# conversion from RawCOPPER or RawDataBlock to RawTOP
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

# dump headers
drdh = register_module(WaveformDumper())
main.add_module(drdh)

# Process all events
process(main)
