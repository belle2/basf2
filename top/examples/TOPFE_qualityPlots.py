#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Jan and Alyssa's waveform analysis script for FE data
# Jan Strube, Sam Cunliffe, Alyssa Loos

from basf2 import *
from ROOT import Belle2, gROOT
from ROOT import TNtuple, TFile, TH1F, TCanvas, TGraph, gStyle
import numpy as np
import argparse
parser = argparse.ArgumentParser(description='Extracts information about the FE parameters',
                                 usage='%(prog)s [options]')
parser.add_argument('filename', help='name of data file')
parser.add_argument(
    '--plotWaveforms',
    action='store_true',
    help='whether to print out suspicious waveforms'
)
parser.add_argument(
    '--output',
    help='name of the ntuple file',
    default="TOPFE.root"
)

args = parser.parse_args()

set_log_level(LogLevel.INFO)
gROOT.SetBatch()


def wf_display(waveform, run, event, suffix=""):

    # Some of the following code was taken from top/tools/showFEWaveforms.py
    # TODO: Think about re-using this code in the two scripts
    '''
    Simple event display of waveforms with feature extraction points
    '''
    #: 1D histograms
    hist = TH1F('h', 'wf', 64 * 4, 0.0, 64.0 * 4)
    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)

    # plot aesthetics
    hist.SetXTitle("sample number")
    hist.SetYTitle("sample value [ADC counts]")
    hist.SetLabelSize(0.06, "XY")   # readable labels
    hist.SetTitleSize(0.07, "XY")   # readable axis titles
    gStyle.SetTitleSize(0.07, "t")  # readable plot title
    c1.SetLeftMargin(0.2)           #
    c1.SetBottomMargin(0.16)        # move margins to make space for axis titles
    c1.SetRightMargin(0.04)         #
    hist.SetTitleOffset(1.3, "Y")   # readable axis offset
    hist.SetTitleOffset(1.0, "X")   # readable axis offset

    #: graphs
    graph = TGraph(10)
    #: output file name
    fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
    pdfFile = fname
    chan = waveform.getChannel()
    asic = waveform.getASICNumber()
    carr = waveform.getCarrierNumber()
    bstk = (chan // 8 // 4 // 4) % 4
    pdfFile = pdfFile + '-' + str(chan)
    nHits = 0
    wf = waveform.getWaveform()
    hist.Reset()
    numSamples = waveform.getSize()
    hist.SetBins(numSamples, 0.0, float(numSamples))
    title = 'channel: ' + str(chan % 8) + ' asic: ' + str(asic) \
        + ' carrier: ' + str(carr) + ' BS: ' + str(bstk) + ' win:'
    for window in waveform.getStorageWindows():
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


class WaveformAnalyzer(Module):
    '''
    Analyzes waveform and FE data
    '''

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''

        #: output ntuple
        self.feProps = TNtuple(
            "feProps",
            "feProps",
            "event:run"
            ":fePeak1Ht:fePeak1TDC:fePeak1Wd"
            ":fePeakHt:fePeakTDC:fePeakWd:fePeakIntegral"
            ":nTOPRawDigits:ch:nNarrowPeaks:nInFirstWindow:nHeight150:nOscillations")

        #: number of waveforms processed
        self.nWaveForms = 0

        #: number of waveforms out of order
        self.nWaveFormsOutOfOrder = 0

        #: object of output file
        self.f = TFile.Open(args.output, "RECREATE")

        #: counts how many plots were created. Stop after 10
        self.plotCounter = 0

    def event(self):
        '''
        Event processor: get data and print to screen
        '''
        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        event = evtMetaData.obj().getEvent()
        run = evtMetaData.obj().getRun()
        waveforms = Belle2.PyStoreArray('TOPRawWaveforms')
        for waveform in waveforms:
            chan = waveform.getChannel()
            self.nWaveForms += 1
            # declare some counters for our ntuple
            nInFirstWindow, nNarrowPeaks, nHeight150, nOscillations = 0, 0, 0, 0

            # waveform.areWindowsInOrder is a bit too strict at the moment
            wins = np.array(waveform.getStorageWindows())
            if not np.all(wins[:-1] <= wins[1:]):  # change false for pics
                self.nWaveFormsOutOfOrder += 1
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "windowOrder")
                    self.plotCounter += 1

            wf = np.array(waveform.getWaveform())
            if False and args.plotWaveforms:
                wf_display(waveform, run, event, "general")
                self.plotCounter += 1

            # If TOPWaveformFeatureExtractor has been run then there are extra
            # TOPRawDigits related to this waveform
            rawDigits = waveform.getRelationsWith("TOPRawDigits")
            nTOPRawDigits = len(rawDigits)

            if False and args.plotWaveforms and nTOPRawDigits > 2:
                # There are at least 3 Top raw digits
                wf_display(waveform, run, event, "manyPeaks")
                self.plotCounter += 1
            if False and args.plotWaveforms and nTOPRawDigits > 3:
                # There are at least 4 TOPRawDigits
                wf_display(waveform, run, event, "tooManyPeaks")
                self.plotCounter += 1
            raw = rawDigits[0]
            fePeakHt = raw.getValuePeak()
            fePeakTDC = raw.getSamplePeak()
            fePeakWd = raw.getSampleFall() - raw.getSampleRise()
            fePeakIntegral = raw.getIntegral()
            # we are sorting the peaks. The FE should always point to the highest one

            # the channel we get from the waveform encodes the asic, carrier, and the actual channel
            if 0 < fePeakTDC < 65 and chan % 8 == 0:
                # To see if cal pulse is in first window
                nInFirstWindow += 1
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "calPuls_firstWin")
                    self.plotCounter += 1

            # there are strange bumps in the ADC distribtion of the cal channels
            if (140 < fePeakHt < 220) and chan % 8 == 0:
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "strangeADCBump_1")
                    self.plotCounter += 1
            if (300 < fePeakHt < 410) and chan % 8 == 0:
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "strangeADCBump_2")
                    self.plotCounter += 1

            fePeak1Ht = -1
            fePeak1TDC = -1
            fePeak1Wd = -1
            fePeak1Integral = -1
            if nTOPRawDigits > 1:
                # if there is a second TOPRawDigit then get it for our ntuple
                fePeak1Ht = rawDigits[1].getValuePeak()
                fePeak1TDC = rawDigits[1].getSamplePeak()
                fePeak1Wd = rawDigits[1].getSampleFall() - rawDigits[1].getSampleRise()
                fePeak1Integral = rawDigits[1].getIntegral()

            if nTOPRawDigits > 1 and fePeak1TDC < 64:
                # counting the times that the second digit is found in the first window
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "secondPeakInFirstWindow")
                    self.plotCounter += 1

            if fePeakWd < 5 or nTOPRawDigits > 1 and fePeak1Wd < 5:
                # counting thin peaks
                nNarrowPeaks += 1
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "thinpeak")
                    self.plotCounter += 1

            if 145 < fePeak1Ht < 155 and chan % 8 == 0:
                # Jan needs to write a comment about why we're doing this test
                nHeight150 += 1
                if False and args.plotWaveforms:
                    wf_display(waveform, run, event, "peak1Ht_is_150")
                    self.plotCounter += 1

            if nTOPRawDigits > 5 and (fePeakHt - fePeak1Ht) < 5:
                if np.mean(wf) < 10:
                    # want to count the portion of times we see this oscillation behavior
                    # guard the call to numpy.mean by the previous if statements
                    nOscillations += 1
                    if False and args.plotWaveforms:
                        wf_display(waveform, run, event, "oscillations")
                        self.plotCounter += 1

            self.feProps.Fill(
                event,
                run,
                fePeak1Ht,
                fePeak1TDC,
                fePeak1Wd,
                fePeakHt,
                fePeakTDC,
                fePeakWd,
                fePeakIntegral,
                nTOPRawDigits,
                chan,
                nNarrowPeaks,
                nInFirstWindow,
                nHeight150,
                nOscillations,
            )

            # only plot the first 10 figures.
            if args.plotWaveforms and self.plotCounter >= 10:
                evtMetaData.obj().setEndOfData()

    def terminate(self):
        '''End of run'''
        self.f.WriteTObject(self.feProps)
        self.f.Close()
        print("# Waveforms:", self.nWaveForms)
        print("out of order:", self.nWaveFormsOutOfOrder)


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
# Create path
main = create_path()
reader = register_module('SeqRootInput')
# file to read
reader.param('inputFileName', args.filename)
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


topfe = register_module('TOPWaveformFeatureExtractor')
# topfe.param('threshold', 20)
main.add_module(topfe)


drdh = register_module(WaveformAnalyzer())
main.add_module(drdh)

# Process all events
process(main)
