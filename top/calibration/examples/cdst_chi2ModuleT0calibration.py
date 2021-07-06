#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------------
# Calibrate module T0 with Bhabha's or dimuons using time difference between slots
#
# usage: basf2 cdst_chi2ModuleT0calibration.py experiment runFirst runLast
#   job: bsub -q l "basf2 cdst_chi2ModuleT0calibration.py experiment runFirst runLast"
#
# note: runLast is inclusive
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
from ROOT import TFile, TH1F, TH2F, TF1, TMatrixDSym
from array import array
import math
import sys
import glob
import os

# ----- those need to be adjusted before running --------------------------------------
#
sampleType = 'bhabha'  # sample type: 'bhabha' or 'dimuon'
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009_nofilter'
skim_dir = 'skim/hlt_bhabha/cdst/sub00/'
globalTag = 'data_reprocessing_prompt'  # base global tag
stagingTags = ['staging_data_reprocessing']  # list of tags with new calibration
localDB = []  # list of local databases with new calibration
minEntries = 10  # minimal number of histogram entries required to perform a fit
output_dir = 'moduleT0'  # main output folder
#
# -------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])

expNo = 'e' + '{:0=4d}'.format(experiment)

# Make list of files
files = []
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    for typ in ['4S', 'Continuum', 'Scan']:
        folder = data_dir + '/' + expNo + '/' + typ + '/' + runNo + '/' + skim_dir
        files += glob.glob(folder + '/cdst.*.root')
if len(files) == 0:
    b2.B2ERROR('No cdst files found')
    sys.exit()

# Output folder
method = 'chi2'
output_folder = output_dir + '/' + expNo + '/' + sampleType + '/' + method
if not os.path.isdir(output_folder):
    os.makedirs(output_folder)
    print('New folder created: ' + output_folder)

# Output file name
fileName = output_folder + '/moduleT0-' + expNo + '-'
run1 = 'r' + '{:0=5d}'.format(run_first)
run2 = 'r' + '{:0=5d}'.format(run_last)
fileName += run1 + '_to_' + run2 + '.root'
print('Output file:', fileName)


class Mask_BS13d(b2.Module):
    ''' exclude (mask-out) BS 13d '''

    def event(self):
        ''' event processing '''

        for digit in Belle2.PyStoreArray('TOPDigits'):
            if digit.getModuleID() == 13 and digit.getBoardstackNumber() == 3:
                digit.setHitQuality(Belle2.TOPDigit.c_Junk)


class ModuleT0cal(b2.Module):
    ''' module T0 calibrator using chi2 minimization of time differences between slots '''

    def initialize(self):
        ''' initialize and book histograms '''

        Belle2.PyStoreArray('TOPTimeZeros').isRequired()

        #: module T0 from database
        self.db_moduleT0 = Belle2.PyDBObj('TOPCalModuleT0')

        #: histograms of time difference between two slots
        self.h_deltaT0 = {}

        #: histogram of number of entries per slot pairs
        self.h_slotPairs = TH2F("slots", "slot pairs: number of entries",
                                16, 0.5, 16.5, 16, 0.5, 16.5)
        self.h_slotPairs.SetXTitle("first slot number")
        self.h_slotPairs.SetYTitle("second slot number")

        #: histogram to store Chi2/ndf of successfully fitted time differences
        self.h_slotPairs_acc = TH2F("slots_acc",
                                    "slot pairs: #chi^{2} /NDF of " +
                                    "successfully fitted time differences",
                                    16, 0.5, 16.5, 16, 0.5, 16.5)
        self.h_slotPairs_acc.SetXTitle("first slot number")
        self.h_slotPairs_acc.SetYTitle("second slot number")

        #: histogram to store minimization results (relative module T0)
        self.h_relModuleT0 = TH1F("relModuleT0", "Module T0 relative to calibration",
                                  16, 0.5, 16.5)
        self.h_relModuleT0.SetXTitle("slot number")
        self.h_relModuleT0.SetYTitle("module T0 residual [ns]")

        #: histogram to store final results (module T0)
        self.h_moduleT0 = TH1F("moduleT0", "Module T0", 16, 0.5, 16.5)
        self.h_moduleT0.SetXTitle("slot number")
        self.h_moduleT0.SetYTitle("module T0 [ns]")

    def sortedTimeZeros(self, unsortedPyStoreArray):
        ''' sorting timeZeros according to slot number '''

        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray]
        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getModuleID(),
            )
        )

    def event(self):
        ''' event processor: fill histograms for events with two entries in timeZeros '''

        timeZeros = Belle2.PyStoreArray('TOPTimeZeros')
        if timeZeros.getEntries() != 2:
            return

        tZeros = self.sortedTimeZeros(timeZeros)

        slot1 = tZeros[0].getModuleID()
        slot2 = tZeros[1].getModuleID()
        self.h_slotPairs.Fill(slot1, slot2)
        if slot1 not in self.h_deltaT0:
            self.h_deltaT0[slot1] = {}
        if slot2 not in self.h_deltaT0[slot1]:
            name = 'deltaT0_' + str(slot1) + '-' + str(slot2)
            title = 'time difference: slot ' + str(slot1) + ' - slot ' + str(slot2)
            self.h_deltaT0[slot1][slot2] = TH1F(name, title, 200, -5, 5)
            self.h_deltaT0[slot1][slot2].SetXTitle('time difference [ns]')
        dt = tZeros[0].getTime() - tZeros[1].getTime()
        self.h_deltaT0[slot1][slot2].Fill(dt)

    def singleFit(self, h):
        ''' perform a fit of time difference distribution '''

        xmin = h.GetXaxis().GetXmin()
        xmax = h.GetXaxis().GetXmax()
        fun = TF1("fun", "[0]*exp(-0.5*((x-[1])/[2])**2)+[3]", xmin, xmax)
        fun.SetParameter(0, h.GetMaximum())
        fun.SetParameter(1, h.GetBinCenter(h.GetMaximumBin()))
        fun.SetParameter(2, 0.2)
        fun.SetParameter(3, 0)
        return h.Fit(fun, "LERSQ")

    def fitHistograms(self, minEntries):
        ''' fit all histograms and store the results into lists '''

        #: fitted time differences
        self.delT0 = []
        #: uncertainties of fitted time differences
        self.sigma = []
        #: matrix denoting which slot pair was involved
        self.A = []
        #: list of histograms selected for output
        self.h_out = []

        for slot1 in range(1, 10):
            if slot1 not in self.h_deltaT0:
                continue
            for slot2 in range(slot1 + 7, slot1 + 10):
                if slot2 not in self.h_deltaT0[slot1]:
                    continue
                h = self.h_deltaT0[slot1][slot2]
                self.h_out.append(h)
                if h.GetEntries() > minEntries:
                    result = self.singleFit(h)
                    if int(result) == 0:
                        chi2 = result.Chi2() / result.Ndf()
                        self.h_slotPairs_acc.SetBinContent(slot1, slot2, chi2)
                        self.delT0.append(result.Parameter(1))
                        self.sigma.append(result.ParError(1))
                        a = [0 for i in range(16)]
                        a[slot1 - 1] = 1
                        a[slot2 - 1] = -1
                        self.A.append(a)

    def minimize(self):
        '''
        Minimization procedure. For the method see NIM A 586 (2008) 174-179, sect. 2.2.
        '''

        # append the bound (sum of all calibration constants equals to 0)
        self.delT0.append(0.0)
        self.sigma.append(0.001)  # arbitrary but small compared to calibration precision
        a = [1 for i in range(16)]
        self.A.append(a)
        m = len(self.delT0)

        # for i, a in enumerate(self.A):
        #    print(a, round(self.delT0[i], 3), round(self.sigma[i], 3))

        # construct the matrix of a linear system of equations
        B = TMatrixDSym(16)
        for i in range(16):
            for j in range(16):
                for k in range(m):
                    B[i][j] += self.A[k][i] * self.A[k][j] / self.sigma[k]**2

        # invert the matrix
        det = array('d', [0])
        B.Invert(det)
        if det[0] == 0:
            b2.B2ERROR("Matrix inversion failed")
            return False

        # construct the right side of a linear system of equations
        b = [0.0 for i in range(16)]
        for i in range(16):
            for k in range(m):
                b[i] += self.A[k][i] * self.delT0[k] / self.sigma[k]**2

        # solve for unknown relative module T0
        x = [0 for i in range(16)]  # module T0's
        e = [0 for i in range(16)]  # uncertainties on module T0
        for i in range(16):
            for j in range(16):
                x[i] += B[i][j] * b[j]
            e[i] = math.sqrt(B[i][i])

        # calculate chi^2 and ndf
        chi2 = 0
        for k in range(m):
            s = 0
            for i in range(16):
                s += self.A[k][i] * x[i]
            chi2 += ((s - self.delT0[k]) / self.sigma[k])**2
        ndf = m - 16
        chi_ndf = 'chi^2/NDF = ' + str(round(chi2, 2)) + '/' + str(ndf)
        self.h_relModuleT0.SetTitle(self.h_relModuleT0.GetTitle() + ' (' + chi_ndf + ')')

        # store relative module T0 in a histogram
        for i in range(16):
            self.h_relModuleT0.SetBinContent(i+1, x[i])
            self.h_relModuleT0.SetBinError(i+1, e[i])

        # calculate arithmetic average of current calibration constants
        T0 = [0.0 for i in range(16)]
        average = 0.0
        num = 0
        for i in range(16):
            if self.db_moduleT0.isCalibrated(i+1):
                T0[i] = self.db_moduleT0.getT0(i+1)
                average += T0[i]
                num += 1
        if num > 0:
            average /= num

        # add current calibration to relative one and subtract the average, then store
        for i in range(16):
            self.h_moduleT0.SetBinContent(i+1, x[i] + T0[i] - average)
            self.h_moduleT0.SetBinError(i+1, e[i])

        b2.B2RESULT("Module T0 calibration constants successfully determined, " + chi_ndf)
        return True

    def terminate(self):
        ''' fit histograms, minimize and write results to file '''

        # fit time difference distributions
        self.fitHistograms(minEntries)

        # minimize
        OK = self.minimize()

        # open output file
        file = TFile.Open(fileName, 'recreate')

        # write histograms and close the file
        self.h_slotPairs.Write()
        self.h_slotPairs_acc.Write()
        if OK:
            self.h_relModuleT0.Write()
            self.h_moduleT0.Write()
        for h in self.h_out:
            h.Write()
        file.Close()

        b2.B2RESULT("Output written to " + fileName)


# Database
b2.use_central_database(globalTag)
for tag in stagingTags:
    b2.use_central_database(tag)
for db in localDB:
    if os.path.isfile(db):
        b2.use_local_database(db, invertLogging=True)
    else:
        b2.B2ERROR(db + ": local database not found")
        sys.exit()

# Create paths
main = b2.create_path()

# Input (cdst files)
main.add_module('RootInput', inputFileNames=files)

# Initialize TOP geometry parameters
main.add_module('TOPGeometryParInitializer')

# Time Recalibrator
main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)

# Channel masking
main.add_module('TOPChannelMasker')

# Exclude BS13d
main.add_module(Mask_BS13d())

# Bunch finder
main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

# Module T0 calibrator
main.add_module(ModuleT0cal())

# Print progress
main.add_module('Progress')

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
