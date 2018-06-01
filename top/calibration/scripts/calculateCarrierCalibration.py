#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TProfile, TFile
from ROOT import gROOT, AddressOf
import math
import ROOT
import sys

gROOT.ProcessLine('struct TreeStruct {\
   int channel;  /* channel number */\
   int slot;     /* module ID */\
   double t0Const; /* local t0 */\
   double t0ConstError; /* uncertainty on local t0 */\
};')

from ROOT import TreeStruct


class calculateCarrierCalibration(Module):
    """
    ** Description **
    A module to identify discerete, 48 ns time shifts in the TOP front-end electronics using collison
    data and correct the localT0 calibration constant accordingly.
    The localT0 constants are read from the DB, modified adding  47.163878 (one write-window) if the carrier is
    flagged as shifted, and saved in a root tree

    **Output**
    * Histograms of the average time per carrier
    * Histograms of the shifted carriers
    * a root tree with the modified localT0 constants.

    **Usage**
    basf2 calibrateCarriers.py -i file1 -i file2 ...

    **Contributors**
    * Marko Staric
    * Umberto Tamponi
    """

    def initialize(self):
        """
        Initializes the module, creating the output tree, the histgrams neede for the shift identification,
        and loading the DB and data objects
        """

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        run = evtMetaData.getRun()
        exp = evtMetaData.getExperiment()

        #: root TFile object
        self.file = ROOT.TFile('localT0WithCarrierCorrection_' + str(exp) + '_run' + str(run) + '.root', 'recreate')
        #: root TTree object
        self.tree = ROOT.TTree('chT0', '')
        #: tree structure
        self.data = TreeStruct()

        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/D'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

        #: profile histograms: time vs channel, one per slot
        self.profiles = [TProfile('prof_' + str(i + 1), 'slot ' + str(i + 1),
                                  16, 0., 512., -1000., 10000.) for i in range(16)]
        #: local T0 calibration constants
        self.localT0 = Belle2.PyDBObj('TOPCalChannelT0')

    def event(self):
        """
        Loops over the TOPdigits ofthe event and fills the carrier time histograms
        """

        recBunch = Belle2.PyStoreObj('TOPRecBunch')
        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            if digit.hasStatus(Belle2.TOPDigit.c_EventT0Subtracted):
                digit.subtractT0(-recBunch.getTime())
                digit.removeStatus(Belle2.TOPDigit.c_EventT0Subtracted)
            if digit.hasStatus(Belle2.TOPDigit.c_OffsetSubtracted):
                digit.subtractT0(-recBunch.getAverageOffset())
                digit.removeStatus(Belle2.TOPDigit.c_OffsetSubtracted)
            if digit.getHitQuality() == 1:
                slot = digit.getModuleID()
                chan = digit.getChannel()
                time = digit.getTime()
                prof = self.profiles[slot - 1]
                prof.Fill(chan, time)

    def terminate(self):
        """
        Analizes the time histograms, identifies the shifter carriers and corrects the constants
        """

        if not self.localT0.isValid():
            B2ERROR('TOPCalChannelT0 payload not valid in terminate')
            return

        mean = 0
        n = 0
        for prof in self.profiles:
            for i in range(prof.GetNbinsX()):
                if prof.GetBinError(i + 1) > 0:
                    mean += prof.GetBinContent(i + 1)
                    n += 1
        mean /= n
        print('mean =', mean)
        low = 0
        n_low = 0
        high = 0
        n_high = 0
        for prof in self.profiles:
            for i in range(prof.GetNbinsX()):
                if prof.GetBinError(i + 1) > 0:
                    y = prof.GetBinContent(i + 1)
                    if y < mean:
                        low += y
                        n_low += 1
                    else:
                        high += y
                        n_high += 1
        if n_low > 0:
            low /= n_low
        print('low =', low)
        if n_high > 0:
            high /= n_high
        print('high =', high)
        cut = (low + high) / 2
        print('cut =', cut)

        histAll = TH1F('histAll', ' whole detector', 16 * 16, 0., 512. * 16)

        for k, prof in enumerate(self.profiles):
            prof.Write()
            slot = k + 1
            hist = TH1F('hist_' + str(slot), 'slot ' + str(slot), 16, 0., 512.)
            for i in range(prof.GetNbinsX()):
                if prof.GetBinError(i + 1) > 0 and prof.GetBinContent(i + 1) > cut:
                    hist.SetBinContent(i + 1, 1.0)
                    histAll.SetBinContent(16 * k + i + 1, 1.0)
            hist.Write()
            for i in range(16):
                shift = hist.GetBinContent(i + 1) * 47.163878  # TODO: get number form DB
                for j in range(32):
                    chan = j + i * 32
                    self.data.channel = chan
                    self.data.slot = slot
                    self.data.t0Const = self.localT0.getT0(slot, chan) + shift
                    self.data.t0ConstError = self.localT0.getT0Error(slot, chan)
                    self.file.cd()
                    self.tree.Fill()
        histAll.Write()
        self.file.Write()
        self.file.Close()


# Create path
main = create_path()

# input
roinput = register_module('RootInput')
main.add_module(roinput)

# calibrate
calibrator = calculateCarrierCalibration()
main.add_module(calibrator)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
