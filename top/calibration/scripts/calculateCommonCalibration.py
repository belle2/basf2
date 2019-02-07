#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TH2F, TFile, TTree, TF1
from array import array
import math


class SetGlobalTagFromFile(Module):
    '''
    set global tag from the input file (any side effects?)
    this module must be positioned just after RootInput
    '''

    def initialize(self):
        """
        reads global tags from the input file and sets central database
        """

        fileMetaData = Belle2.PyStoreObj('FileMetaData', Belle2.DataStore.c_Persistent)
        globalTags = fileMetaData.getDatabaseGlobalTag().split(',')
        print("Global tags: ", globalTags)
        for tag in reversed(globalTags):
            use_central_database(tag)
            B2RESULT('Add global tag: ' + tag)


class calibrateGlobalT0Offline(Module):
    """
    ** Description **
    Module to calculate the commont time offset of the TOP detector.
    It selects two-track events, saves the bunch finder time offset in an histrogram and fits it to
    extract the global time offset with respect to the RF clock. It should be run in an individual
    job for each run, but in future it may be further authomatize to run on multiple runs
    in the same job.

    **Output**
    Root file containing:
    * The histogram of the bunch finder time offset
    * The fit function
    * The fit parameters ina tree format (usefuly when merging several files)

    **Usage**
    basf2 calibrateGlobalT0Offline -i file1 -i file2 ...

    **Contributors**
    * Marko Staric
    * Umberto Tamponi
    """

    def initialize(self):
        """
        Creates the histogram used for the commoT0 calculation, and
        takes the necessary objects from the DataStore
        """

        geo = Belle2.PyDBObj('TOPGeometry')
        if not geo.isValid():
            B2FATAL('TOP geometry not available in database')

        #: bunch separation time
        self.bunchTimeSep = geo.getNominalTDC().getSyncTimeBase() / 24

        #: histogram of current offset
        self.h1 = TH1F("offset", "current offset; offset [ns]", 600, -9.0, 9.0)

        #: histogram of current offset vs event number
        self.h2 = TH2F("offset_vs_event", "current offset versus event number",
                       100, 0.0, 1000000.0, 200, -3.0, 3.0)
        self.h2.SetXTitle("event number")
        self.h2.SetYTitle("offset [ns]")

        xmi = -self.bunchTimeSep / 2
        xma = self.bunchTimeSep / 2
        #: histogram of current offset, wrap-around into [-1/2, 1/2] of bunch sep. time
        self.h1a = TH1F("offset_a", "current offset; offset [ns]", 200, xmi, xma)

        #: histogram of current offset vs event number, wrap-around into [-1/2, 1/2]
        self.h2a = TH2F("offset_vs_event_a", "current offset versus event number",
                        100, 0.0, 1000000.0, 200, xmi, xma)
        self.h2a.SetXTitle("event number")
        self.h2a.SetYTitle("offset [ns]")

        xmi = 0.0
        xma = self.bunchTimeSep
        #: histogram of current offset, wrap-around into [0, 1] of bunch sep. time
        self.h1b = TH1F("offset_b", "current offset; offset [ns]", 200, xmi, xma)

        #: histogram of current offset vs event number, wrap-around into [0, 1]
        self.h2b = TH2F("offset_vs_event_b", "current offset versus event number",
                        100, 0.0, 1000000.0, 200, xmi, xma)
        self.h2b.SetXTitle("event number")
        self.h2b.SetYTitle("offset [ns]")

        evtMetaData = Belle2.PyStoreObj('EventMetaData')

        #: experiment number
        self.expNo = evtMetaData.getExperiment()

        #: run number formatted as string with leading zeros
        self.run = '{:0=5d}'.format(evtMetaData.getRun())

        #: common T0 used for the calibration of the input file
        self.t0 = 0
        #: common T0 uncertainty
        self.t0Err = 0
        commonT0 = Belle2.PyDBObj('TOPCalCommonT0')
        if commonT0.isValid():
            if commonT0.isCalibrated():
                self.t0 = commonT0.getT0()
                self.t0Err = commonT0.getT0Error()
                B2INFO('Common T0 used in data processing: ' + str(self.t0))
            else:
                B2INFO('No common T0 calibration done yet')
        else:
            B2ERROR('Common T0 not available in database')

    def event(self):
        """
        Fills the histogram of the time offset
        """
        recBunch = Belle2.PyStoreObj('TOPRecBunch')
        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        if not recBunch.isValid():
            return
        if recBunch.isReconstructed() and recBunch.getNumTracks() == 2:
            offset = recBunch.getCurrentOffset()
            evtNum = evtMetaData.getEvent()
            self.h1.Fill(offset)
            self.h2.Fill(evtNum, offset)
            # wrap-around into [-1/2, 1/2] of bunch cycle
            a = offset - round(offset / self.bunchTimeSep, 0) * self.bunchTimeSep
            self.h1a.Fill(a)
            self.h2a.Fill(evtNum, a)
            # wrap-around into [0, 1] of bunch cycle
            b = offset - round(offset / self.bunchTimeSep - 0.5, 0) * self.bunchTimeSep
            self.h1b.Fill(b)
            self.h2b.Fill(evtNum, b)

    def getHistogramToFit(self):
        """
        Selects a histogram with the peak closest to histogram center
        """

        halfbins = self.h1a.GetNbinsX() / 2
        if abs(self.h1a.GetMaximumBin() - halfbins) < abs(self.h1b.GetMaximumBin() - halfbins):
            return self.h1a
        else:
            return self.h1b

    def terminate(self):
        """
        Performs the fit of the bunch finder offset distribution, using a line + gaussian function.
        The results of the fit are saved in a tree
        """

        # Get histogram to fit
        h_to_fit = self.getHistogramToFit()
        maximum = h_to_fit.GetBinCenter(h_to_fit.GetMaximumBin())
        hmax = h_to_fit.GetMaximum()
        sigma0 = 0.16
        xmin = h_to_fit.GetXaxis().GetXmin()
        xmax = h_to_fit.GetXaxis().GetXmax()

        # Fit function
        func = TF1(
            'func',
            '([0]/TMath::Sqrt(2*TMath::Pi() * [2]*[2])) *exp(-0.5*((x-[1])/[2])**2) + [3]*x + [4]',
            xmin, xmax)
        func.SetParameter(0, hmax * 0.9 * math.sqrt(2*math.pi) * sigma0)
        func.SetParameter(1, maximum)
        func.SetParameter(2, sigma0)
        func.SetParLimits(2, 0.05, 0.3)  # to avoid the sign ambiguity
        func.SetParameter(3, 0.)
        func.SetParameter(4, hmax * 0.1)
        h_to_fit.Fit(func, 'L R S')

        # Tree creation, branches declaration....
        fileName = 'commonT0Histo-run' + str(self.run) + '.root'

        outFile = TFile(fileName, 'recreate')
        tree = TTree('tree', '')

        expNum = array('i', [0])
        runNum = array('i', [0])
        fitted_offset = array('f', [0.])
        offset = array('f', [0.])
        offsetErr = array('f', [0.])
        sigma = array('f', [0.])
        integral = array('f', [0.])
        nEvt = array('f', [0.])
        chi2 = array('f', [0.])

        tree.Branch('expNum', expNum, 'expNum/I')
        tree.Branch('runNum', runNum, 'runNum/I')
        tree.Branch('fitted_offset', fitted_offset, 'fitted_offset/F')
        tree.Branch('offset', offset, 'offset/F')
        tree.Branch('offsetErr', offsetErr, 'offsetErr/F')
        tree.Branch('sigma', sigma, 'sigma/F')
        tree.Branch('chi2', chi2, 'chi2/F')
        tree.Branch('integral', integral, 'integral/F')
        tree.Branch('nEvt', nEvt, 'nEvt/F')

        # Dumps the fit results into the tree
        expNum[0] = self.expNo
        runNum[0] = int(self.run)
        fitted_offset[0] = func.GetParameter(1)
        offset[0] = fitted_offset[0] + self.t0
        offsetErr[0] = func.GetParError(1)
        sigma[0] = func.GetParameter(2)
        integral[0] = func.GetParameter(0) / h_to_fit.GetBinWidth(1)
        chi2[0] = func.GetChisquare() / float(func.GetNDF())
        nEvt[0] = h_to_fit.Integral()

        tree.Fill()
        tree.Write()
        self.h1.Write()
        self.h1a.Write()
        self.h1b.Write()
        self.h2.Write()
        self.h2a.Write()
        self.h2b.Write()
        outFile.Close()

        B2RESULT('Calibration for exp' + str(self.expNo) + '-run' + self.run)
        if self.t0Err > 0:
            B2RESULT('Old common T0 [ns]: ' + str(round(self.t0, 3)) + ' +- ' +
                     str(round(self.t0Err, 3)))
        else:
            B2RESULT('Old common T0 [ns]: -- not calibrated -- ')
        B2RESULT('New common T0 [ns]: ' + str(round(offset[0], 3)) + ' +- ' +
                 str(round(offsetErr[0], 3)))

# Create path
main = create_path()

# input
roinput = register_module('RootInput')
roinput.param('branchNames', ['TOPRecBunch'])
main.add_module(roinput)

# set global tag from the input file
main.add_module(SetGlobalTagFromFile())

# calibrate
main.add_module(calibrateGlobalT0Offline())

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
