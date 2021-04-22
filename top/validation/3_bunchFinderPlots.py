#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>EvtGenSimRec.root, EvtGenSimRec_B2Kpi.root</input>
  <output>bunchFinder.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>makes validation plots for TOPBunchFinder</description>
</header>
"""

from basf2 import create_path, process, statistics, Module
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed
import math


class MakePlots(Module):
    '''
    Makes validation histograms for bunch finder
    '''

    def set_descr(self, h, D):
        '''
        Sets description, check and contact to validation histogram.
        :param h validation histogram
        :param D description text
        '''
        descr = TNamed('Description', D)
        h.GetListOfFunctions().Add(descr)
        check = TNamed('Check', 'Must agree with reference')
        h.GetListOfFunctions().Add(check)
        contact = TNamed('Contact', 'marko.staric@ijs.si')
        h.GetListOfFunctions().Add(contact)

    def initialize(self):
        ''' Initializes the Module: book histograms and set descriptions and checks'''

        #: root file pointer
        self.tfile = TFile.Open("bunchFinder.root", "recreate")

        #: validation histogram
        self.recBunchNo = TH1F("recBunchNo",
                               "Reconstructed relative bunch number; relative bunch number; events per bunch",
                               100, -50.0 - 0.5, 50.0 - 0.5)
        self.set_descr(self.recBunchNo, 'Reconstructed bunch number relative to L1 trigger.')

        #: validation histogram
        self.offset = TH1F("offset",
                           "Offset to reconstructed bunch; offset [ns]; events per bin",
                           100, -1.0, 1.0)
        self.set_descr(self.offset, 'Offset to reconstructed bunch.')

        #: validation histogram
        self.numTracks = TH1F("numTracks",
                              "Number of tracks in TOP acceptance; number of tracks; entries per bin",
                              10, 0.5, 10.5)
        self.set_descr(self.numTracks, 'Number of tracks per event in the acceptance of TOP.')

        #: validation histogram
        self.usedTracks = TH1F("usedTracks",
                               "Number of tracks used for bunch finding; number of used track; entries per bins",
                               10, 0.5, 10.5)
        self.set_descr(self.usedTracks, 'Number of tracks in the event actually used for finding the bunch crossing.')

        #: validation histogram
        self.effi_numTracks = TH1F("effi_numTracks",
                                   "Efficiency vs. number of tracks; number of tracks; efficiency",
                                   10, 0.5, 10.5)
        self.set_descr(self.effi_numTracks, 'Efficieny of finding the correct bunch crossing versus '
                       'the number of tracks per event in the acceptance of TOP.')

        #: validation histogram
        self.effi_usedTracks = TH1F("effi_usedTracks",
                                    "Efficiency vs. number of used tracks; number of used tracks; efficiency",
                                    10, 0.5, 10.5)
        self.set_descr(self.effi_usedTracks, 'Efficieny of finding the correct bunch crossing versus '
                       'the number of tracks per event actually used.')

        #: validation histogram
        self.effi = TH1F("effi", "Acceptance and efficiency", 2, 0.5, 2.5)
        self.effi.GetXaxis().SetBinLabel(1, "acceptance")
        self.effi.GetXaxis().SetBinLabel(2, "efficiency")
        self.effi.GetXaxis().SetLabelSize(0.08)
        self.effi.GetXaxis().SetAlphanumeric()
        self.set_descr(self.effi, 'Acceptance is defined as fraction of events with at least one track crossing TOP. '
                       'Efficiency is defined as fraction of correctly reconstructed bunch crossings in accepted events.')

        #: number of events with valid TOPRecBunch object pointer
        self.nev = 0

    def event(self):

        ''' Event processor: fill histograms '''

        recBunch = Belle2.PyStoreObj('TOPRecBunch')
        if not recBunch.isValid():
            return
        self.nev += 1
        if recBunch.isReconstructed():
            self.recBunchNo.Fill(recBunch.getBunchNo())
            self.offset.Fill(recBunch.getCurrentOffset())
            self.numTracks.Fill(recBunch.getNumTracks())
            self.usedTracks.Fill(recBunch.getUsedTracks())
            if recBunch.getBunchNo() == recBunch.getMCBunchNo():
                self.effi_numTracks.Fill(recBunch.getNumTracks())
                self.effi_usedTracks.Fill(recBunch.getUsedTracks())

    def terminate(self):
        ''' Processes and writes histograms to file'''

        eff = self.numTracks.GetEntries() / self.nev
        err = math.sqrt(eff * (1 - eff) / self.nev)
        self.effi.SetBinContent(1, eff)
        self.effi.SetBinError(1, err)

        eff = self.effi_numTracks.GetEntries() / self.numTracks.GetEntries()
        err = math.sqrt(eff * (1 - eff) / self.numTracks.GetEntries())
        self.effi.SetBinContent(2, eff)
        self.effi.SetBinError(2, err)

        self.numTracks.Sumw2()
        self.effi_numTracks.Sumw2()
        self.effi_numTracks.Divide(self.effi_numTracks, self.numTracks, 1, 1, "B")

        self.usedTracks.Sumw2()
        self.effi_usedTracks.Sumw2()
        self.effi_usedTracks.Divide(self.effi_usedTracks, self.usedTracks, 1, 1, "B")

        self.tfile.Write()
        self.tfile.Close()


# Create path
main = create_path()

# Input
main.add_module('RootInput', inputFileNames=['../EvtGenSimRec_B2Kpi.root', '../EvtGenSimRec.root'])

# Make plots
main.add_module(MakePlots())

# Process events
process(main)

# Print call statistics
print(statistics)
