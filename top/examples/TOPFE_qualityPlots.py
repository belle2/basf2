#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# debug script to dump raw header information
#
# (loosely) based on Root2Binary.py
######################################################

from basf2 import *
from ROOT import Belle2
from ROOT import TNtuple, TFile
import sys
import h5py
import numpy as np
import peakutils

argvs = sys.argv


def wf_display(waveform):
    '''
    Simple event display of waveforms with feature extraction points
    '''
    k = 0
    nHits = 0
    fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
    self.pdfFile = fname
    chan = waveform.getChannel()
    self.pdfFile = self.pdfFile + '-' + str(chan)
    wf = waveform.getWaveform()
    self.hist[k].Reset()
    numSamples = waveform.getSize()
    self.hist[k].SetBins(numSamples, 0.0, float(numSamples))
    title = 'chan ' + str(chan) + ' win'
    for window in waveform.getReferenceWindows():
        title += ' ' + str(window)
    self.hist[k].SetTitle(title)
    self.hist[k].SetStats(False)
    self.hist[k].SetLineColor(4)
    if not waveform.areWindowsInOrder():
        self.hist[k].SetLineColor(3)
    i = 0
    for sample in wf:
        i = i + 1
        self.hist[k].SetBinContent(i, sample)

    rawDigits = waveform.getRelationsWith("TOPRawDigits")
    i = 0
    for raw in rawDigits:
        self.graph[k].SetPoint(i, raw.getSampleRise() + 0.5, raw.getValueRise0())
        i += 1
        self.graph[k].SetPoint(i, raw.getSampleRise() + 1.5, raw.getValueRise1())
        i += 1
        self.graph[k].SetPoint(i, raw.getSamplePeak() + 0.5, raw.getValuePeak())
        i += 1
        self.graph[k].SetPoint(i, raw.getSampleFall() + 0.5, raw.getValueFall0())
        i += 1
        self.graph[k].SetPoint(i, raw.getSampleFall() + 1.5, raw.getValueFall1())
        i += 1
        if raw.isFEValid():  # works properly only for single rawDigit!
            self.graph[k].SetMarkerColor(2)
            if raw.isPedestalJump():
                self.graph[k].SetMarkerColor(3)
        else:
            self.graph[k].SetMarkerColor(4)
        self.graph[k].Set(i)

        k = k + 1
        if k == 4:
            stat = self.draw(k, event, run, self.hist)
            if stat:
                return
            k = 0
            self.pdfFile = fname

    if k > 0:
        self.draw(k, event, run, self.hist)
    #: 1D histograms
    hist = [TH1F('h' + str(i), 'wf', 64 * 4, 0.0, 64.0 * 4) for i in range(4)]
    for i in range(4):
        hist[i].SetXTitle("sample number")
        hist[i].SetYTitle("sample value [ADC counts]")
        hist[i].SetLabelSize(0.06, "XY")
        hist[i].SetTitleSize(0.07, "XY")
        hist[i].SetTitleOffset(0.7, "XY")
    #: graphs
    graph = [TGraph(10) for i in range(4)]
    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)
    #: output file name
    pdfFile = 'waveforms'

    def wait(self):
        ''' wait for user respond '''

        try:
            q = 0
            Q = 0
            p = 1
            P = 1
            abc = eval(input('Type <CR> to continue, P to print or Q to quit '))
            if abc == 1:
                self.pdfFile = self.pdfFile + '.pdf'
                self.c1.SaveAs(self.pdfFile)
                print('Canvas saved to file:', self.pdfFile)
                return False
            else:
                evtMetaData = Belle2.PyStoreObj('EventMetaData')
                evtMetaData.obj().setEndOfData()
                return True
        except:
            return False

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''

        self.c1.Divide(1, 4)
        self.c1.Show()

    def draw(self, k, event, run, hist):
        ''' Draw histograms and wait for user respond '''

        self.c1.Clear()
        self.c1.Divide(1, 4)
        title = 'WF event display:' + ' run ' + str(run) + ' event ' \
            + str(event)
        self.c1.SetTitle(title)
        for i in range(k):
            self.c1.cd(i + 1)
            self.hist[i].Draw()
            self.graph[i].SetMarkerStyle(24)
            self.graph[i].Draw("sameP")
        self.c1.Update()
        stat = self.wait()
        return stat


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

    def event(self):
        '''
        Event processor: get data and print to screen
        '''
        waveforms = Belle2.PyStoreArray('TOPRawWaveforms')
        for waveform in waveforms:
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
            idx = sorted(wf[indexes])
            pyPeak0Ht = -1
            if nPyPeaks > 0:
                pyPeak0Ht = idx[0]
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
