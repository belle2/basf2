#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Display of waveforms with feature extraction points superimposed
# Unpacker is set for Interim FE format v2.1
# Usage: basf2 showFEWaveforms.py -i <file_name.sroot>
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH1F, TCanvas, TGraph


class WFDisplay(Module):

    '''
    Simple event display of waveforms with feature extraction points
    '''

    #: 1D histograms
    hist = [TH1F('h' + str(i), 'wf', 64*4, 0.0, 64.0*4) for i in range(4)]
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
            self.graph[i].SetMarkerColor(2)
            self.graph[i].Draw("sameP")
        self.c1.Update()
        stat = self.wait()
        return stat

    def event(self):
        '''
        Event processor: fill the histograms, display them,
        wait for user respond.
        '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        event = evtMetaData.obj().getEvent()
        run = evtMetaData.obj().getRun()

        waveforms = Belle2.PyStoreArray('TOPRawWaveforms')

        k = 0
        nHits = 0
        fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
        self.pdfFile = fname
        for waveform in waveforms:
            chan = waveform.getChannel()
            self.pdfFile = self.pdfFile + '-' + str(chan)
            wf = waveform.getWaveform()
            self.hist[k].Reset()
            numSamples = waveform.getSize()
            self.hist[k].SetBins(numSamples, 0.0, float(numSamples))
            title = 'chan ' + str(chan) + ' win '
            title += str(waveform.getStorageWindow())
            self.hist[k].SetTitle(title)
            self.hist[k].SetStats(False)
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


set_log_level(LogLevel.INFO)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

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

# Display waveforms
main.add_module(WFDisplay())

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
