#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -------------------------------------------------------------------------------------------
# example of comparing time vs channel before and after t0 calibration
# input: "t0CalTime_slot<#>.root" from t0LaserCalData.py
# Usage: basf2 t0LaserCalData.py -i <TOPDigits.root> <slot#>
#        TOPDigits.root is the output file of unpackToTOPDigitsWithTBC.py
# -------------------------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2
import ROOT
from ROOT import gStyle, gROOT, AddressOf, TH2F


gStyle.SetOptStat(0)
gROOT.ProcessLine('struct TreeStruct {\
   int slot;\
   int channel;\
   int pixel;\
   int pmt;\
   float time;\
   float caltime;\
   float height;\
   float width;\
   int quality;\
   float rawtime;\
   int firstwindow;\
   int pmtpixel;\
};')

from ROOT import TreeStruct

from sys import argv
args = sys.argv


class Ntuple(Module):
    ''' t0const ntpule infomation '''

    global t0const
    #: t0 constant per channel per slot
    t0const = {0: 0.0}

    #: input t0const root file
    f = ROOT.TFile.Open('t0const_slot' + str(args[1]) + '.root')
    for event in f.chT0:
        t0const[event.channel] = event.t0Const

    #: scatter plot before t0 calibartion
    histTimeCh = TH2F('before T0Cal slot#' + str(args[1]), 'before T0Cal slot#' + str(args[1]), 512, 0, 511, 500, 50, 100)
    #: scatter plot after t0 calibartion
    histCalTimeCh = TH2F('after T0Cal slot#' + str(args[1]), 'after T0Cal slot#' + str(args[1]), 512, 0, 511, 500, 50, 100)

    histTimeCh.GetXaxis().SetTitle('channel')
    histTimeCh.GetYaxis().SetTitle('time [ns]')
    histCalTimeCh.GetXaxis().SetTitle('channel')
    histCalTimeCh.GetYaxis().SetTitle('time [ns]')

    def initialize(self):
        ''' Initialize the Module: output root file '''

        #: output file name
        self.file = ROOT.TFile('t0CalTime_slot' + str(args[1]) + '.root', 'recreate')
        #: output tree name
        self.tree = ROOT.TTree('laser', '')
        #: tree strruct
        self.data = TreeStruct()

        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def event(self):
        ''' Event processor: fill the tree and scatter plots '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            if digit.getModuleID() == int(args[1]):
                self.data.slot = digit.getModuleID()
                self.data.channel = digit.getChannel()
                self.data.pixel = digit.getPixelID()
                self.data.time = digit.getTime()
                self.data.caltime = digit.getTime() + t0const.get(digit.getChannel())
                self.data.height = digit.getPulseHeight()
                self.data.width = digit.getPulseWidth()
                self.data.quality = digit.getHitQuality()
                self.data.rawtime = digit.getRawTime()
                self.data.firstwindow = digit.getFirstWindow()
                self.data.pmtpixel = digit.getPMTPixel()
                self.data.pmt = digit.getPMTNumber()

                self.file.cd()
                self.tree.Fill()

                self.histTimeCh.Fill(digit.getChannel(), digit.getTime())
                self.histCalTimeCh.Fill(digit.getChannel(), digit.getTime() - t0const.get(digit.getChannel()))

    def terminate(self):
        ''' Write the file '''

        self.file.cd()
        self.file.Write()
        self.histTimeCh.Write()
        self.histCalTimeCh.Write()
        self.file.Close()


# Create path
main = create_path()

# input
main.add_module('RootInput')

# write to ntuple
main.add_module(Ntuple())

# Show progress of processing
main.add_module('Progress')

# Process events
process(main)

# Print statistics
print(statistics)
