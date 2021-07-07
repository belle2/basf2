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
# Unpacker is set for Interim FE format v2.1
# Usage: basf2 showFEWaveforms.py -i <file_name.sroot>
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False  # noqa

import basf2 as b2
from ROOT import Belle2
from ROOT import TH1F, TCanvas, TGraph


class WFDisplay(b2.Module):

    '''
    Simple event display of waveforms with feature extraction points
    '''

    #: 1D histograms
    hist = [TH1F('h' + str(i), 'wf', 64 * 4, 0.0, 64.0 * 4) for i in range(4)]
    for i in range(4):
        hist[i].SetXTitle("sample number")
        hist[i].SetYTitle("sample value [ADC counts]")
        hist[i].SetLabelSize(0.06, "XY")
        hist[i].SetTitleSize(0.07, "XY")
        hist[i].SetTitleOffset(0.7, "XY")
    #: graphs for painting waveforms after the gap in window number
    gpaint = [TGraph() for i in range(4)]
    #: graphs for FE points
    graphs = [[TGraph(5)] for i in range(4)]
    #: graphs for template fit points
    tlpfgraphs = [[TGraph(2)] for i in range(4)]
    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)
    #: output file name
    pdfFile = 'waveforms'

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''

        self.c1.Divide(1, 4)
        self.c1.Show()
        print('WARNING: Template Fit is still under development and currently returns wrong position for rising edge!.')

    def wait(self):
        ''' wait for user respond '''

        user_input = input("Press Enter to continue, P to print, or Q to quit ").lower().strip()
        if user_input == "p":
            filename = self.pdfFile + '.pdf'
            self.c1.SaveAs(filename)
            print('Canvas saved to file:', filename)
            return False
        elif user_input == "q":
            evtMetaData = Belle2.PyStoreObj('EventMetaData')
            evtMetaData.obj().setEndOfData()
            return True
        else:
            return False

    def draw(self, k, event, run):
        ''' Draw histograms and wait for user respond '''

        self.c1.Clear()
        self.c1.Divide(1, 4)
        title = 'WF event display:' + ' run ' + str(run) + ' event ' \
            + str(event)
        self.c1.SetTitle(title)
        for i in range(k):
            self.c1.cd(i + 1)
            self.hist[i].Draw()
            if self.gpaint[i].GetN() > 0:
                self.gpaint[i].Draw("same")
            for graph in self.graphs[i]:
                graph.Draw("sameP")
            for tlpfgraph in self.tlpfgraphs[i]:
                tlpfgraph.Draw("sameP")
        self.c1.Update()
        stat = self.wait()
        return stat

    def set_gpaint(self, waveform, k):
        ''' construct a graph to paint waveform differently after window discontinuity '''

        if waveform.areWindowsInOrder():
            self.gpaint[k].Set(0)

        windows = waveform.getStorageWindows()
        i0 = windows.size()
        for i in range(1, windows.size()):
            diff = windows[i] - windows[i - 1]
            if diff < 0:
                diff += 512
            if diff != 1:
                i0 = i
                break
        n = (windows.size() - i0) * 64 * 2
        self.gpaint[k].Set(n)
        low = i0 * 64
        for i in range(low, self.hist[k].GetNbinsX()):
            x = self.hist[k].GetBinCenter(i + 1)
            dx = self.hist[k].GetBinWidth(i + 1) / 2
            y = self.hist[k].GetBinContent(i + 1)
            ii = (i - low) * 2
            self.gpaint[k].SetPoint(ii, x - dx, y)
            self.gpaint[k].SetPoint(ii + 1, x + dx, y)

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
        # nHits = 0
        fname = 'waveforms_run' + str(run) + '_event' + str(event) + '_chan'
        #: output file name
        self.pdfFile = fname
        for waveform in waveforms:
            slot = waveform.getModuleID()
            chan = waveform.getChannel()
            self.pdfFile = self.pdfFile + '-S' + str(slot) + '_' + str(chan)
            wf = waveform.getWaveform()
            self.hist[k].Reset()
            numSamples = waveform.getSize()
            startSample = waveform.getStartSample()
            self.hist[k].SetBins(numSamples, float(startSample), float(startSample + numSamples))
            title = 'S' + str(slot) + ' chan ' + str(chan) + ' win'
            if waveform.getStorageWindows().empty():
                title += ' ' + str(waveform.getStorageWindow())
            else:
                for window in waveform.getStorageWindows():
                    title += ' ' + str(window)
            self.hist[k].SetTitle(title)
            self.hist[k].SetStats(False)
            self.hist[k].SetLineColor(8)

            i = 0
            for sample in wf:
                i = i + 1
                self.hist[k].SetBinContent(i, sample)

            self.gpaint[k].SetLineColor(9)
            self.set_gpaint(waveform, k)

            rawDigits = waveform.getRelationsWith("TOPRawDigits")
            self.graphs[k].clear()
            self.tlpfgraphs[k].clear()
            for raw in rawDigits:
                graph = TGraph(5)
                graph.SetMarkerStyle(24)
                graph.SetPoint(0, raw.getSampleRise() + 0.5, raw.getValueRise0())
                graph.SetPoint(1, raw.getSampleRise() + 1.5, raw.getValueRise1())
                graph.SetPoint(2, raw.getSamplePeak() + 0.5, raw.getValuePeak())
                graph.SetPoint(3, raw.getSampleFall() + 0.5, raw.getValueFall0())
                graph.SetPoint(4, raw.getSampleFall() + 1.5, raw.getValueFall1())
                if raw.isMadeOffline():
                    graph.SetMarkerStyle(5)
                if raw.isFEValid() and not raw.isAtWindowDiscontinuity():
                    graph.SetMarkerColor(2)
                    if raw.isPedestalJump():
                        graph.SetMarkerColor(3)
                else:
                    graph.SetMarkerColor(4)
                self.graphs[k].append(graph)

                tlpfResult = raw.getRelated("TOPTemplateFitResults")
                if tlpfResult:
                    tlpfgraph = TGraph(2)
                    tlpfgraph.SetMarkerStyle(25)
#                    tlpfgraph.SetPoint(0, tlpfResult.getMean(), tlpfResult.getAmplitude() + tlpfResult.getBackgroundOffset())
#                    tlpfgraph.SetPoint(1, tlpfResult.getMean(), tlpfResult.getBackgroundOffset())
                    print('Template Fit Chisquare: ', tlpfResult.getChisquare())
                    self.tlpfgraphs[k].append(tlpfgraph)

            k = k + 1
            if k == 4:
                stat = self.draw(k, event, run)
                if stat:
                    return
                k = 0
                self.pdfFile = fname

        if k > 0:
            self.draw(k, event, run)


b2.set_log_level(b2.LogLevel.INFO)

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP (needed only in PocketDAQ)
rawconverter = b2.register_module('Convert2RawDet')
main.add_module(rawconverter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Add multiple hits from waveforms
featureExtractor = b2.register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
main.add_module(converter)

# Display waveforms
main.add_module(WFDisplay())

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
