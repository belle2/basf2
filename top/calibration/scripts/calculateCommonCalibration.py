#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TH2F, TFile, TTree, TF1
from array import array


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
    basf2 calibrateGlobalT0Offline -i file1 -ifile2 ...

    **Contributors**
    * Marko Staric
    * Umberto Tamponi
    """

    def initialize(self):
        """
        Creates the histogram used for the commoT0 calculation, and
        takes the necessary objects from the DataStore
        """
        self.h1 = TH1F("offset", "current offset; offset [ns]", 600, -9.0, 9.0)
        self.h2 = TH2F("offset_vs_event", "current offset versus event number",
                       100, 0.0, 1000000.0, 200, -3.0, 3.0)
        self.h2.SetXTitle("event number")
        self.h2.SetYTitle("offset [ns]")
        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        self.run = '{:0=5d}'.format(evtMetaData.getRun())

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

    def terminate(self):
        """
        Performs the fit of the bunch finder offset distribution, using a line + gaussian function.
        The fit range is +/- 1.5 ns, centered authomatically around the maximum of the offset histogram
        The results of the fit are saved in a tree
        """

        maximum = self.h1.GetBinCenter(self.h1.GetMaximumBin())

        # Fit function
        func = TF1(
            'func',
            '([0]/TMath::Sqrt(2*TMath::Pi() * [2]*[2])) *exp(-0.5*((x-[1])/[2])**2) + [3]*x + [4]',
            maximum -
            1.5,
            maximum +
            1.5)
        func.SetParameter(0, 1.)
        func.SetParameter(1, maximum)
        func.SetParameter(2, 0.16)
        func.SetParLimits(2, 0.09, 0.2)  # to avoid the sign ambiguity
        func.SetParameter(3, 1.)
        func.SetParameter(4, 1.)
        self.h1.Fit(func, 'L R S')

        # Tree creation, branches declaration....
        fileName = 'commonT0Histo-run' + str(self.run) + '.root'

        outFile = TFile(fileName, 'recreate')
        tree = TTree('tree', '')

        runNum = array('i', [0])
        offset = array('f', [0.])
        offsetErr = array('f', [0.])
        sigma = array('f', [0.])
        integral = array('f', [0.])
        nEvt = array('f', [0.])
        chi2 = array('f', [0.])

        tree.Branch('runNum', runNum, 'runNum/I')
        tree.Branch('offset', offset, 'offset/F')
        tree.Branch('offsetErr', offsetErr, 'offsetErr/F')
        tree.Branch('sigma', sigma, 'sigma/F')
        tree.Branch('chi2', chi2, 'chi2/F')
        tree.Branch('integral', integral, 'integral/F')
        tree.Branch('nEvt', nEvt, 'nEvt/F')

        nEvt = self.h1.Integral()

        # Dumps the fit results into the tree
        offset[0] = func.GetParameter(1)
        offsetErr[0] = func.GetParError(1)
        sigma[0] = func.GetParameter(2)
        integral[0] = func.GetParameter(0) / 0.03  # bin size in ns
        chi2[0] = func.GetChisquare() / float(func.GetNDF())
        runNum[0] = int(self.run)

        tree.Fill()
        tree.Write()
        self.h1.Write()
        self.h2.Write()
        outFile.Close()


# Create path
main = create_path()

# input
roinput = register_module('RootInput')
roinput.param('branchNames', ['TOPRecBunch'])
main.add_module(roinput)

# calibrate
main.add_module(calibrateGlobalT0Offline())

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
