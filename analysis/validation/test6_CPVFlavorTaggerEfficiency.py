#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>CPVToolsOutput.root</input>
  <output>test6_CPVFlavorTaggerEfficiency.root</output>
  <contact>Fernando Abudinen; abudinen@mpp.mpg.de</contact>
  <description>This file calculates the effective efficiency of the category based flavor tagger considering the two
  standard combiners and the individual categories. Validation plots are also pruduced. </description>
</header>
"""


import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
from basf2 import *
from flavorTagger import *
import pdg
import basf2_mva
import math
import re
import subprocess
import copy
from string import Template
from modularAnalysis import *
# from ROOT import PyConfig
# PyConfig.IgnoreCommandLineOptions = True
# PyConfig.StartGuiThread = False
from array import array
import shutil

ROOT.gROOT.SetBatch(True)

workingFile = str("../CPVToolsOutput.root")
workingFiles = glob.glob(str(workingFile))
treeName = str("B0tree")

if len(workingFiles) < 1:
    sys.exit("No file name or file names " + str(workingFile) + " found.")

workingDirectory = '.'

#
# *****************************************
# DETERMINATION OF TOTAL EFFECTIVE EFFIENCY
# *****************************************
#

r_subsample = array('d', [
    0.0,
    0.1,
    0.25,
    0.5,
    0.625,
    0.75,
    0.875,
    1.0])
r_size = len(r_subsample)
average_eff = 0

# working directory
# needs the B0_B0bar_final.root-file
# treeName = 'B0tree'


# All possible Categories
categories = [
    'Electron',
    'IntermediateElectron',
    'Muon',
    'IntermediateMuon',
    'KinLepton',
    'IntermediateKinLepton',
    'Kaon',
    'KaonPion',
    'SlowPion',
    'FSC',
    'MaximumPstar',
    'FastHadron',
    'Lambda']


methods = []


class Quiet:
    """Context handler class to quiet errors in a 'with' statement"""

    def __init__(self, level=ROOT.kInfo + 1):
        """Class constructor"""
        #: the level to quiet
        self.level = level

    def __enter__(self):
        """Enter the context"""
        #: the previously set level to be ignored
        self.oldlevel = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = self.level

    def __exit__(self, type, value, traceback):
        """Exit the context"""
        ROOT.gErrorIgnoreLevel = self.oldlevel


tree = ROOT.TChain(treeName)

mcstatus = array('d', [-511.5, 0.0, 511.5])
# ROOT.TH1.SetDefaultSumw2()

for iFile in workingFiles:
    # if Belle2.FileSystem.findFile(workingFile):
    tree.AddFile(iFile)

totalBranches = []
for branch in tree.GetListOfBranches():
    totalBranches.append(branch.GetName())

if 'B0_FBDT_qrCombined' in totalBranches:
    methods.append("FBDT")

if 'B0_FANN_qrCombined' in totalBranches:
    methods.append("FANN")

usedCategories = []
for cat in categories:
    catBranch = 'B0_qp' + cat
    if catBranch in totalBranches:
        usedCategories.append(cat)

if len(usedCategories) > 1:
    WhichCategories(usedCategories)

categoriesNtupleList = str()
for (particleList, category, combinerVariable) in eventLevelParticleLists:
    categoriesNtupleList = categoriesNtupleList + "Eff_%s:" % category


# Output Validation file
outputFile = ROOT.TFile("test6_CPVFlavorTaggerEfficiency.root", "RECREATE")

# Values to be watched
outputNtuple = ROOT.TNtuple(
    "FT_Efficiencies",
    "Effective efficiencies of the flavor tagger combiners as well as of the individual tagging categories.",
    "Eff_FBDT:DeltaEff_FBDT:Eff_FANN:DeltaEff_FANN:" + categoriesNtupleList)

outputNtuple.SetAlias('Description', "These are the effective efficiencies of the flavor tagger combiners as well as of " +
                      "the individual tagging efficiencies.")
outputNtuple.SetAlias(
    'Check',
    "These values should not change drastically. Since the nightly reconstruction validation runs" +
    "on the same input file (which changes only from release to release), the values between builds should be the same.")
outputNtuple.SetAlias('Contact', "abudinen@mpp.mpg.de")

efficienciesForNtuple = []

YmaxForQrPlot = 0

for method in methods:
    # bekommt man mit GetBinError(), setzten mit SetBinError()
    # histogram contains the average r in each of 6 bins -> calculation see below
    histo_avr_r = ROOT.TH1F('Average_r', 'Average r in each of 6 bins (B0 and B0bar)', 6,
                            r_subsample)
    histo_avr_rB0 = ROOT.TH1F('Average_rB0', 'Average r in each of 6 bins (B0)', 6,
                              r_subsample)
    histo_avr_rB0bar = ROOT.TH1F('Average_rB0bar', 'Average r in each of 6 bins (B0bar)', 6,
                                 r_subsample)
    # histogram with number of entries in for each bin
    histo_entries_per_bin = ROOT.TH1F(
        'entries_per_bin',
        'Events binned in r_subsample according to their r-value for B0 and B0bar prob',
        6,
        r_subsample)
    histo_entries_per_binB0 = ROOT.TH1F('entries_per_binB0', 'Events binned in r_subsample according '
                                        'to their r-value for B0 prob', 6, r_subsample)
    histo_entries_per_binB0bar = ROOT.TH1F('entries_per_binB0bar',
                                           'Events binned in r_subsample according to their r-value '
                                           'for B0bar prob', 6, r_subsample)
    # histogram network output (not qr and not r) for true B0 (signal) - not necessary
    histo_Cnet_output_B0 = ROOT.TH1F('Comb_Net_Output_B0', 'Combiner network output [not equal to r] '
                                     'for true B0 (binning 100)', 100, 0.0, 1.0)
    # histogram network output (not qr and not r) for true B0bar (background) - not necessary
    histo_Cnet_output_B0bar = ROOT.TH1F('Comb_Net_Output_B0bar', 'Combiner network output [not equal to r] '
                                        'for true B0bar (binning 100)', 100, 0.0, 1.0)
    # histogram containing the belle paper plot (qr-tagger output for true B0)
    histo_belleplotB0 = ROOT.TH1F('qr_' + method + '_B0',
                                  'BellePlot for true B0 (binning 50)', 50,
                                  -1.0, 1.0)
    # histogram containing the belle paper plot (qr-tagger output for true B0bar)
    histo_belleplotB0bar = ROOT.TH1F('qr_' + method + '_B0Bar',
                                     'BellePlot for true B0Bar (binning 50)',
                                     50, -1.0, 1.0)

    # histogram containing the qr plot (qr-tagger output)
    histo_belleplotBoth = ROOT.TH1F('qr_' + method + '_B0Both',
                                    'qr-tagger output (binning 50)',
                                    50, -1.0, 1.0)
    # calibration plot for B0. If we get a linaer line our MC is fine, than the assumption r ~ 1- 2w is reasonable
    # expectation is, that for B0 calibration plot:  qr=0  half B0 and half B0bar, qr = 1 only B0 and qr = -1
    # no B0. Inverse for B0bar calibration plot
    histo_calib_B0 = ROOT.TH1F('Calibration_' + method + '_B0', 'CalibrationPlot for true B0', 100, -1.0, 1.0)
    # calibration plot for B0bar calibration plot
    histo_calib_B0bar = ROOT.TH1F('Calibration_' + method + '_B0Bar',
                                  'CalibrationPlot for true B0Bar', 100, -1.0,
                                  1.0)
    # belle plot with true B0 and B0bars
    hallo12 = ROOT.TH1F('BellePlot_NoCut', 'BellePlot_NoCut (binning 100)',
                        100, -1.0, 1.0)

    ###############################
    diag = ROOT.TF1('diag', 'pol1', -1, 1)
    ###############################

    # histograms for the efficiency calculation in wrong way
    histo_m0 = ROOT.TH1F('BellePlot_B0_m0',
                         'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m1 = ROOT.TH1F('BellePlot_B0_m1',
                         'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m2 = ROOT.TH1F('BellePlot_B0_m2',
                         'BellePlot_m for true B0Bar (binning 50)', 50, -1.0,
                         1.0)

    # filling the histograms

    tree.Draw('B0_' + method + '_qrCombined>>qr_' + method + '_B0', 'B0_qrMC == 1')
    tree.Draw('B0_' + method + '_qrCombined>>qr_' + method + '_B0Bar', 'B0_qrMC == -1')
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_NoCut', 'abs(B0_qrMC) == 1')
    tree.Draw('B0_' + method + '_qrCombined>>qr_' + method + '_B0Both', 'abs(B0_qrMC) == 1')

    tree.Draw('B0_' + method + '_qrCombined>>Calibration_' + method + '_B0', 'B0_qrMC == 1')
    tree.Draw('B0_' + method + '_qrCombined>>Calibration_' + method + '_B0Bar', 'B0_qrMC == -1')

    # filling histograms wrong efficiency calculation
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_B0_m0',
              'B0_qrMC == 1 && B0_' + method + '_qrCombined>0')
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_B0_m1',
              'B0_qrMC == 1 && B0_' + method + '_qrCombined<0')
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_B0_m2',
              'B0_qrMC == -1 && B0_' + method + '_qrCombined>0 ')

    # filling with abs(qr) in one of 6 bins with its weight
    # separate calculation for B0 and B0bar

    tree.Project('Average_r', 'abs(B0_' + method + '_qrCombined)',
                 'abs(B0_' + method + '_qrCombined)')
    tree.Project('Average_rB0', 'abs(B0_' + method + '_qrCombined)', 'abs(B0_' + method + '_qrCombined)*(B0_qrMC==1)')
    tree.Project('Average_rB0bar', 'abs(B0_' + method + '_qrCombined)', 'abs(B0_' + method + '_qrCombined)*(B0_qrMC==-1)')

    # filling with abs(qr) in one of 6 bins
    tree.Project('entries_per_bin', 'abs(B0_' + method + '_qrCombined)', 'abs(B0_qrMC) == 1')
    tree.Project('entries_per_binB0', 'abs(B0_' + method + '_qrCombined)', 'B0_qrMC == 1')
    tree.Project('entries_per_binB0bar', 'abs(B0_' + method + '_qrCombined)', 'B0_qrMC == -1')

    # producing the average r histograms
    histo_avr_r.Divide(histo_entries_per_bin)
    histo_avr_rB0.Divide(histo_entries_per_binB0)
    histo_avr_rB0bar.Divide(histo_entries_per_binB0bar)

    # producing the calibration plots
    # Errors ok
    histo_calib_B0.Divide(hallo12)
    histo_calib_B0bar.Divide(hallo12)

    # Fit for calibration plot
    print(' ')
    print('****************** CALIBRATION CHECK FOR COMBINER USING ' + method + ' ***************************************')
    print(' ')
    print('Fit ploynomial of first order to the calibration plot. Expected value ~0.5')
    print(' ')
    histo_calib_B0.Fit(diag, 'TEST')
    print('       ')
    print('****************** MEASURED EFFECTIVE EFFICIENCY FOR COMBINER USING ' + method + ' ***************************')
    print('*                                                                                                  *')
    # get total number of entries
    total_entries = histo_entries_per_bin.GetEntries()
    total_entries_B0 = histo_entries_per_binB0.GetEntries()
    total_entries_B0bar = histo_entries_per_binB0bar.GetEntries()
    tot_eff_effB0 = 0
    tot_eff_effB0bar = 0
    event_fractionB0 = array('f', [0] * r_size)
    event_fractionB0bar = array('f', [0] * r_size)
    event_fractionTotal = array('f', [0] * r_size)
    event_fractionDiff = array('f', [0] * r_size)
    rvalueB0 = array('f', [0] * r_size)
    rvalueB0bar = array('f', [0] * r_size)
    rvalueB0Average = array('f', [0] * r_size)
    wvalue = array('f', [0] * r_size)
    wvalueB0 = array('f', [0] * r_size)
    wvalueB0bar = array('f', [0] * r_size)
    wvalueDiff = array('f', [0] * r_size)
    entries = array('f', [0] * r_size)
    entriesB0 = array('f', [0] * r_size)
    entriesB0bar = array('f', [0] * r_size)
    iEffEfficiency = array('f', [0] * r_size)
    iDeltaEffEfficiency = array('f', [0] * r_size)
    # intervallEff = array('f', [0] * r_size)
    performance = []
    for i in range(1, r_size):
        # get the average r-value
        rvalueB0[i] = histo_avr_rB0.GetBinContent(i)
        rvalueB0bar[i] = histo_avr_rB0bar.GetBinContent(i)
        rvalueB0Average[i] = (rvalueB0[i] + rvalueB0bar[i]) / 2
        # calculate the wrong tag fractin (only true if MC data good)
        wvalue[i] = (1 - rvalueB0Average[i]) / 2
        wvalueB0[i] = (1 - rvalueB0[i]) / 2
        wvalueB0bar[i] = (1 - rvalueB0bar[i]) / 2
        wvalueDiff[i] = wvalueB0[i] - wvalueB0bar[i]
        entries[i] = histo_entries_per_bin.GetBinContent(i)
        entriesB0[i] = histo_entries_per_binB0.GetBinContent(i)
        entriesB0bar[i] = histo_entries_per_binB0bar.GetBinContent(i)
        # fraction of events/all events
        event_fractionTotal[i] = (entriesB0[i] + entriesB0bar[i]) / total_entries
        event_fractionDiff[i] = (entriesB0[i] - entriesB0bar[i]) / total_entries
        event_fractionB0[i] = entriesB0[i] / total_entries_B0
        event_fractionB0bar[i] = entriesB0bar[i] / total_entries_B0bar
        iEffEfficiency[i] = (event_fractionB0[i] * rvalueB0[i] * rvalueB0[i] +
                             event_fractionB0bar[i] * rvalueB0bar[i] * rvalueB0bar[i]) / 2
        iDeltaEffEfficiency[i] = event_fractionB0[i] * rvalueB0[i] * \
            rvalueB0[i] - event_fractionB0bar[i] * rvalueB0bar[i] * rvalueB0bar[i]
        # finally calculating the total effective efficiency
        tot_eff_effB0 = tot_eff_effB0 + event_fractionB0[i] * rvalueB0[i] * rvalueB0[i]
        tot_eff_effB0bar = tot_eff_effB0bar + event_fractionB0bar[i] * rvalueB0bar[i] * rvalueB0bar[i]

    average_eff = (tot_eff_effB0 + tot_eff_effB0bar) / 2
    diff_eff = tot_eff_effB0 - tot_eff_effB0bar
    print('* ------------------------------------------------------------------------------------------------ *')
    print('*                                                                                                  *')
    print('*    __________________________________________________________________________________________    *')
    print('*   |                                                                                          |   *')
    print('*   | TOTAL NUMBER OF TAGGED EVENTS  =  ' +
          '{:<24}'.format("%.0f" % total_entries) + '{:>36}'.format('|   *'))
    print('*   |                                                                                          |   *')
    print('*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY  (q=+-1)=  ' + '{:.2f}'.format(average_eff * 100) +
          ' %                                    |   *')
    print('*   |                                                                                          |   *')
    print('*   | B0-TAGGER  TOTAL EFFECTIVE EFFICIENCIES: ' +
          '{:.2f}'.format(tot_eff_effB0 * 100) + ' % (q=+1)  ' +
          '{:.2f}'.format(tot_eff_effB0bar * 100) + ' % (q=-1)  EffDiff=' +
          '{:^5.2f}'.format(diff_eff * 100) + ' % |   *')
    print('*   |                                                                                          |   *')
    print('*   | FLAVOR PERCENTAGE (MC):                  ' +
          '{:.2f}'.format(total_entries_B0 / total_entries * 100) + ' % (q=+1)  ' +
          '{:.2f}'.format(total_entries_B0bar / total_entries * 100) + ' % (q=-1)  Diff=' +
          '{:^5.2f}'.format((total_entries_B0 - total_entries_B0bar) / total_entries * 100) + ' %    |   *')
    print('*   |__________________________________________________________________________________________|   *')
    print('*                                                                                                  *')
    print('****************************************************************************************************')
    print('*                                                                                                  *')

    efficienciesForNtuple.append(float(average_eff * 100))
    efficienciesForNtuple.append(float(diff_eff * 100))

    maxB0 = histo_belleplotB0.GetBinContent(histo_belleplotB0.GetMaximumBin())
    maxB0bar = histo_belleplotB0bar.GetBinContent(histo_belleplotB0bar.GetMaximumBin())
    maxB0Both = histo_belleplotBoth.GetBinContent(histo_belleplotBoth.GetMaximumBin())

    Ymax = max(maxB0, maxB0bar, maxB0Both)
    Ymax = Ymax + Ymax / 12

    if YmaxForQrPlot < Ymax:
        YmaxForQrPlot = Ymax

    # produce a pdf
    ROOT.gStyle.SetOptStat(0)
    with Quiet(ROOT.kError):
        Canvas1 = ROOT.TCanvas('Bla', 'Final Output', 1200, 800)
    Canvas1.cd()  # activate
    Canvas1.SetLeftMargin(0.13)
    Canvas1.SetRightMargin(0.04)
    Canvas1.SetTopMargin(0.03)
    Canvas1.SetBottomMargin(0.14)
    histo_belleplotB0.SetFillColorAlpha(ROOT.kBlue, 0.2)
    histo_belleplotB0.SetFillStyle(1001)
    histo_belleplotB0.GetXaxis().SetLabelSize(0.04)
    histo_belleplotB0.GetYaxis().SetLabelSize(0.04)
    histo_belleplotB0.GetYaxis().SetTitleOffset(0.9)
    histo_belleplotB0.GetXaxis().SetTitleSize(0.06)
    histo_belleplotB0.GetYaxis().SetTitleSize(0.06)
    histo_belleplotB0.GetYaxis().SetLimits(0, YmaxForQrPlot)
    histo_belleplotB0.SetLineColor(ROOT.kBlue)
    histo_belleplotB0bar.SetFillColorAlpha(ROOT.kRed, 1.0)
    histo_belleplotB0bar.SetFillStyle(3005)
    histo_belleplotB0bar.SetLineColor(ROOT.kRed)
    # SetLabelSize etc SetTitle

    histo_belleplotB0.SetTitle('; #it{qr}_{' + method + '} ; Events  (Total = ' + '{:<1}'.format("%.0f" % total_entries) + ')'
                               )
    histo_belleplotB0.SetMinimum(0)
    histo_belleplotB0.SetMaximum(YmaxForQrPlot)
    histo_belleplotB0.Draw('hist')
    histo_belleplotB0bar.Draw('hist same')

    leg = ROOT.TLegend(0.2, 0.7, 0.9, 0.95)
    leg.AddEntry(
        histo_belleplotB0,
        'true B^{0} ' +
        ' #varepsilon_{eff}(B^{0}) = ' +
        '{:.2f}'.format(
            tot_eff_effB0 *
            100) +
        '%    #frac{n_{B^{0}}}{n} = ' +
        '{:.2f}'.format(
            total_entries_B0 /
            total_entries *
            100) +
        '%')
    leg.AddEntry(
        histo_belleplotB0bar,
        'true #bar{B}^{0} ' +
        ' #varepsilon_{eff}(#bar{B}^{0}) = ' +
        '{:.2f}'.format(
            tot_eff_effB0bar *
            100) +
        '%    #frac{n_{#bar{B}^{0}}}{n} = ' +
        '{:.2f}'.format(
            total_entries_B0bar /
            total_entries *
            100) +
        '%')
    leg.AddEntry("", "Avrg.     #bf{  #varepsilon_{eff} = " + '{:.2f}'.format(average_eff * 100) +
                 '%}   #Delta#varepsilon_{eff} = ' + '{:^5.2f}'.format(diff_eff * 100) + '%')
    leg.SetTextSize(0.045)
    leg.Draw()

    Canvas1.Update()

    with Quiet(ROOT.kError):
        Canvas1.SaveAs(workingDirectory + '/' + 'test6_CPVFTqr' + method + '_both.pdf')

    # Validation Plot 1
    histo_belleplotBoth.GetXaxis().SetLabelSize(0.04)
    histo_belleplotBoth.GetYaxis().SetLabelSize(0.04)
    histo_belleplotBoth.GetYaxis().SetTitleOffset(0.7)
    histo_belleplotBoth.GetXaxis().SetTitleOffset(0.7)
    histo_belleplotBoth.GetXaxis().SetTitleSize(0.06)
    histo_belleplotBoth.GetYaxis().SetTitleSize(0.06)

    histo_belleplotBoth.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    histo_belleplotBoth.GetListOfFunctions().Add(ROOT.TNamed('Description', 'Output of the flavor tagger combiner ' + method))
    histo_belleplotBoth.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            'Shape should not change drastically. E.g. Warning if the peak at 0 increases or if the peaks at +-1 decrease.'))
    histo_belleplotBoth.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'abudinen@mpp.mpg.de'))

    histo_belleplotBoth.SetTitle(
        'Flavor tagger output for combiner ' +
        method +
        '; #it{qr}_{' +
        method +
        '} ; Events  (Total = ' +
        '{:<1}'.format(
            "%.0f" %
            total_entries) +
        ')')
    histo_belleplotBoth.SetMinimum(0)
    histo_belleplotBoth.SetMaximum(YmaxForQrPlot)
    histo_belleplotBoth.SetStats(False)
    histo_belleplotBoth.Write()

    # Validation Plot 2
    histo_belleplotB0.GetYaxis().SetTitleOffset(0.7)
    histo_belleplotB0.GetXaxis().SetTitleOffset(0.7)
    histo_belleplotB0.SetLineColor(ROOT.kBlue + 2)
    histo_belleplotB0.SetTitle(
        'Flavor tagger output for combiner ' +
        method +
        ' for true B^{0}s; #it{qr}_{' +
        method +
        '} ; Events  (Total = ' +
        '{:<1}'.format(
            "%.0f" %
            histo_belleplotB0.GetEntries()) +
        ')')
    histo_belleplotB0.SetStats(False)

    histo_belleplotB0.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    histo_belleplotB0.GetListOfFunctions().Add(
        ROOT.TNamed('Description', 'Output of the flavor tagger combiner ' + method + ' for true B0s'))
    histo_belleplotB0.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            'Shape should not change drastically. E.g. Warning if the peak at 0 increases or if the peak at +1 decreases.'))
    histo_belleplotB0.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'abudinen@mpp.mpg.de'))
    histo_belleplotB0.Write()

    # Validation Plot 3
    histo_belleplotB0bar.GetXaxis().SetLabelSize(0.04)
    histo_belleplotB0bar.GetYaxis().SetLabelSize(0.04)
    histo_belleplotB0bar.GetYaxis().SetTitleOffset(0.7)
    histo_belleplotB0bar.GetXaxis().SetTitleOffset(0.7)
    histo_belleplotB0bar.GetXaxis().SetTitleSize(0.06)
    histo_belleplotB0bar.GetYaxis().SetTitleSize(0.06)
    histo_belleplotB0bar.SetLineColor(ROOT.kBlue + 2)
    histo_belleplotB0bar.SetTitle(
        'Flavor tagger output for combiner ' +
        method +
        ' for true #bar{B}^{0}s; #it{qr}_{' +
        method +
        '} ; Events  (Total = ' +
        '{:<1}'.format(
            "%.0f" %
            histo_belleplotB0bar.GetEntries()) +
        ')')
    histo_belleplotB0bar.SetMinimum(0)
    histo_belleplotB0bar.SetMaximum(YmaxForQrPlot)
    histo_belleplotB0bar.SetStats(False)

    histo_belleplotB0bar.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    histo_belleplotB0bar.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Description',
            'Output of the flavor tagger combiner ' +
            method +
            ' for true B0bars'))
    histo_belleplotB0bar.GetListOfFunctions().Add(ROOT.TNamed(
        'Check', 'Shape should not change drastically. E.g. Warning if the peak at 0 increases or if the peak at -1 decreases.'))
    histo_belleplotB0bar.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'abudinen@mpp.mpg.de'))
    histo_belleplotB0bar.Write()

    # IPython.embed()

    # produce the nice calibration plot
    with Quiet(ROOT.kError):
        Canvas2 = ROOT.TCanvas('Bla2', 'Calibration plot for true B0', 1200, 800)
    Canvas2.cd()  # activate
    Canvas2.SetLeftMargin(0.13)
    Canvas2.SetRightMargin(0.04)
    Canvas2.SetTopMargin(0.03)
    Canvas2.SetBottomMargin(0.14)
    histo_calib_B0.GetXaxis().SetLabelSize(0.04)
    histo_calib_B0.GetYaxis().SetLabelSize(0.04)
    histo_calib_B0.GetYaxis().SetTitleOffset(0.9)
    histo_calib_B0.GetXaxis().SetTitleSize(0.06)
    histo_calib_B0.GetYaxis().SetTitleSize(0.06)
    histo_calib_B0.SetFillColorAlpha(ROOT.kBlue, 0.2)
    histo_calib_B0.SetFillStyle(1001)
    histo_calib_B0.GetYaxis().SetTitleOffset(0.9)
    histo_calib_B0.SetLineColor(ROOT.kBlue)

    histo_calib_B0.SetTitle('; #it{qr}_{' + method + '} ; Calibration '
                            )
    histo_calib_B0.Draw('hist')
    diag.Draw('SAME')

    leg2 = ROOT.TLegend(0.2, 0.75, 0.63, 0.93)
    leg2.SetHeader(" y = #it{m}#it{x} + #it{c}", "")
    leg2.GetListOfPrimitives().First().SetTextAlign(22)
    leg2.AddEntry(
        diag,
        "#it{m} = " +
        '{:.2f}'.format(
            diag.GetParameter("p1")) +
        "    #it{c} = " +
        '{:.2f}'.format(
            diag.GetParameter("p0")))
    leg2.SetTextSize(0.05)
    leg2.Draw()

    Canvas2.Update()
    with Quiet(ROOT.kError):
        Canvas2.SaveAs(workingDirectory + '/' + 'test6_CPVFTCalibration_' + method + '_B0.pdf')

    # Validation Plot 4

    histo_calib_B0.GetYaxis().SetTitleOffset(0.7)
    histo_calib_B0.GetXaxis().SetTitleOffset(0.7)
    histo_calib_B0.SetLineColor(ROOT.kBlue + 2)
    histo_calib_B0.SetTitle('Calibration plot for the flavor tagger combiner ' +
                            method + ' ; #it{qr}_{' + method + '} ; Calibration')
    histo_calib_B0.SetMinimum(-0.2)
    histo_calib_B0.SetMaximum(+1.2)
    histo_calib_B0.SetStats(False)

    histo_calib_B0.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    histo_calib_B0.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Description',
            'Calibration plot for the flavor tagger combiner ' +
            method +
            ' for true B0s'))
    histo_calib_B0.GetListOfFunctions().Add(
        ROOT.TNamed('Check', 'Shape should not change drastically. E.g. warning if the shape stops beeing linear.'))
    histo_calib_B0.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'abudinen@mpp.mpg.de'))
    histo_calib_B0.Write()

    histo_belleplotBoth.Delete()
    histo_avr_r.Delete()
    histo_avr_rB0.Delete()
    histo_avr_rB0bar.Delete()
    histo_entries_per_bin.Delete()
    histo_entries_per_binB0.Delete()
    histo_entries_per_binB0bar.Delete()
    histo_Cnet_output_B0.Delete()
    histo_Cnet_output_B0bar.Delete()
    histo_belleplotB0.Delete()
    histo_belleplotB0bar.Delete()
    histo_calib_B0.Delete()
    histo_calib_B0bar.Delete()
    hallo12.Delete()
    histo_m0.Delete()
    histo_m1.Delete()
    histo_m2.Delete()
    Canvas1.Clear()
    Canvas2.Clear()


# **********************************************
# DETERMINATION OF INDIVIDUAL EFFECTIVE EFFICIENCY
# **********************************************

# keep in mind:
# the individual efficiency is determind on basis of the combiner training.
# Whereas the efficiency is determined on basis of the final expert output.

# needs the B0Tagger.root-file from combiner teacher

print('************************* MEASURED EFFECTIVE EFFICIENCY FOR INDIVIDUAL CATEGORIES *********************************')
print('*                                                                                                                 *')
# input: Classifier input from event-level. Output of event-level is recalculated for input on combiner-level.
# but is re-evaluated under combiner target. Signal is B0, background is B0Bar.

for (particleList, category, combinerVariable) in eventLevelParticleLists:
    # histogram of input variable (only signal) - not yet a probability! It's a classifier plot!
    hist_signal = ROOT.TH1F('Signal_' + category, 'Input Signal (B0)' +
                            category + ' (binning 50)', 50, -1.0, 1.0)
    # histogram of input variable (only background) - not yet a probability! It's a classifier plot!
    hist_background = ROOT.TH1F('Background_' + category, 'Input Background (B0bar)' +
                                category + ' (binning 50)', 50, -1.0, 1.0)
    hist_both = ROOT.TH1F('qp_' + category, 'Input Background (B0bar)' +
                          category + ' (binning 50)', 100, -1, 1)

    # per definiton that input is not comparable to the network output, this has to be transformed.
    # probability output from 0 to 1 (corresponds to net output probability) -> calculation below
    hist_probB0 = ROOT.TH1F('ProbabilityB0_' + category,
                            'Transformed to probability (B0) (' + category + ')',
                            50, 0.0, 1.0)
    hist_probB0bar = ROOT.TH1F('ProbabilityB0bar_' + category,
                               'Transformed to probability (B0bar) (' + category + ')',
                               50, 0.0, 1.0)
    # qp output from -1 to 1 -> transformation below
    hist_qpB0 = ROOT.TH1F('QRB0_' + category, 'Transformed to qp (B0)(' +
                          category + ')', 50, -1.0, 1.0)
    hist_qpB0bar = ROOT.TH1F('QRB0bar_' + category, 'Transformed to qp (B0bar) (' +
                             category + ')', 50, -1.0, 1.0)
    # histogram for abs(qp), i.e. this histogram contains the r-values -> transformation below
    # also used to get the number of entries, sorted into 6 bins
    hist_absqpB0 = ROOT.TH1F('AbsQRB0_' + category, 'Abs(qp)(B0) (' + category + ')', 6, r_subsample)
    hist_absqpB0bar = ROOT.TH1F('AbsQRB0bar_' + category, 'Abs(qp) (B0bar) (' + category + ')', 6, r_subsample)
    # histogram contains at the end the average r values -> calculation below
    # sorted into 6 bins
    hist_aver_rB0 = ROOT.TH1F('AverageRB0_' + category, 'A good one (B0)' +
                              category, 6, r_subsample)
    hist_aver_rB0bar = ROOT.TH1F('AverageRB0bar_' + category, 'A good one (B0bar)' +
                                 category, 6, r_subsample)
    # ****** TEST OF CALIBRATION ******
    # for calibration plot we want to have
    hist_all = ROOT.TH1F('All_' + category, 'Input Signal (B0) and Background (B0Bar)' +
                         category + ' (binning 50)', 50, 0.0, 1.0)
    tree.Draw('B0_qp' + category + '>>All_' + category, 'B0_qrMC!=0')
    hist_calib_B0 = ROOT.TH1F('Calib_B0_' + category, 'Calibration Plot for true B0' +
                              category + ' (binning 50)', 50, 0.0, 1.0)
    tree.Draw('B0_qp' + category + '>>Calib_B0_' + category, 'B0_qrMC == 1.0')
    hist_calib_B0.Divide(hist_all)

    # fill signal
    tree.Draw('B0_qp' + category + '>>Signal_' + category, 'B0_qrMC == 1.0')
    # fill background
    tree.Draw('B0_qp' + category + '>>Background_' + category, 'B0_qrMC == -1.0'
              )
    # fill both
    tree.Draw('B0_qp' + category + '>>qp_' + category, 'abs(B0_qrMC) == 1.0'
              )

    # ***** TEST OF CALIBRATION ******

    # initialize some arrays
    purityB0 = array('d', [0] * 51)
    dilutionB02 = array('d', [0] * 51)
    purityB0bar = array('d', [0] * 51)
    dilutionB0bar2 = array('d', [0] * 51)
    signal = array('d', [0] * 51)
    back = array('d', [0] * 51)
    weight = array('d', [0] * 51)

    for i in range(1, 51):
        # doing the transformation to probabilities
        signal[i] = hist_signal.GetBinContent(i)
        back[i] = hist_background.GetBinContent(i)

        weight[i] = signal[i] + back[i]

        # avoid dividing by zero
        if signal[i] + back[i] == 0:
            purityB0[i] = 0
            dilutionB02[i] = 0
            purityB0bar[i] = 0
            dilutionB0bar2[i] = 0
        else:
            purityB0[i] = signal[i] / (signal[i] + back[i])
            dilutionB02[i] = -1 + 2 * signal[i] / (signal[i] + back[i])

            purityB0bar[i] = back[i] / (signal[i] + back[i])
            dilutionB0bar2[i] = -1 + 2 * back[i] / (signal[i] + back[i])

        # filling histogram with probabilty from 0 to 1
        hist_probB0.Fill(purityB0[i], signal[i])
        hist_probB0bar.Fill(purityB0bar[i], back[i])

        # filling histogram with qp from -1 to 1
        hist_qpB0.Fill(dilutionB02[i], signal[i])
        hist_qpB0bar.Fill(dilutionB0bar2[i], back[i])

        # filling histogram with abs(qp), i.e. this histogram contains the r-values (not qp)
        hist_absqpB0.Fill(abs(dilutionB02[i]), signal[i])
        hist_absqpB0bar.Fill(abs(dilutionB0bar2[i]), back[i])
        # filling histogram with abs(qp) special weighted - needed for average r calculation
        hist_aver_rB0.Fill(abs(dilutionB02[i]), abs(dilutionB02[i]) * signal[i])
        hist_aver_rB0bar.Fill(abs(dilutionB0bar2[i]), abs(dilutionB0bar2[i]) * back[i])

    # hist_aver_rB0bar contains now the average r-value
    hist_aver_rB0.Divide(hist_absqpB0)
    hist_aver_rB0bar.Divide(hist_absqpB0bar)
    # now calculating the efficiency

    # calculating number of events
    tot_entriesB0 = 0
    tot_entriesB0bar = 0
    for i in range(1, r_size):
        tot_entriesB0 = tot_entriesB0 + hist_absqpB0.GetBinContent(i)
        tot_entriesB0bar = tot_entriesB0bar + hist_absqpB0bar.GetBinContent(i)
    # initializing some arrays
    tot_eff_effB0 = 0
    tot_eff_effB0bar = 0
    event_fractionB0 = array('f', [0] * r_size)
    event_fractionB0bar = array('f', [0] * r_size)
    rvalueB0 = array('f', [0] * r_size)
    rvalueB0bar = array('f', [0] * r_size)
    # wvalue = array('f', [0] * r_size)
    entriesB0 = array('f', [0] * r_size)
    entriesB0bar = array('f', [0] * r_size)

    for i in range(1, r_size):
        rvalueB0[i] = hist_aver_rB0.GetBinContent(i)
        rvalueB0bar[i] = hist_aver_rB0bar.GetBinContent(i)
        # wvalue[i] = (1 - rvalueB0[i]) / 2
        entriesB0[i] = hist_absqpB0.GetBinContent(i)
        entriesB0bar[i] = hist_absqpB0bar.GetBinContent(i)
        event_fractionB0[i] = entriesB0[i] / tot_entriesB0
        event_fractionB0bar[i] = entriesB0bar[i] / tot_entriesB0bar
        # print '*  Bin ' + str(i) + ' r-value: ' + str(rvalueB0[i]), 'entriesB0: ' +
        # str(event_fractionB0[i] * 100) + ' % (' + str(entriesB0[i]) + '/' +
        # str(tot_entriesB0) + ')'
        tot_eff_effB0 = tot_eff_effB0 + event_fractionB0[i] * rvalueB0[i] \
            * rvalueB0[i]
        tot_eff_effB0bar = tot_eff_effB0bar + event_fractionB0bar[i] * rvalueB0bar[i] \
            * rvalueB0bar[i]
    effDiff = tot_eff_effB0 - tot_eff_effB0bar
    effAverage = (tot_eff_effB0 + tot_eff_effB0bar) / 2

    print(
        '{:<25}'.format("* " +
                        category) +
        ' B0-Eff=' +
        '{: 8.2f}'.format(
            tot_eff_effB0 *
            100) +
        ' %' +
        '   B0bar-Eff=' +
        '{: 8.2f}'.format(
            tot_eff_effB0bar *
            100) +
        ' %' +
        '   EffAverage=' +
        '{: 8.2f}'.format(effAverage * 100) + ' %' +
        '   EffDiff=' +
        '{: 8.2f}'.format(effDiff * 100) + ' %   *')

    # ****** produce the input plots from combiner level ******

    efficienciesForNtuple.append(float(effAverage * 100))

    maxSignal = hist_signal.GetBinContent(hist_signal.GetMaximumBin())
    maxBackground = hist_background.GetBinContent(hist_background.GetMaximumBin())

    Ymax = max(maxSignal, maxBackground)
    Ymax = Ymax + Ymax / 12

    ROOT.gStyle.SetOptStat(0)
    with Quiet(ROOT.kError):
        Canvas = ROOT.TCanvas('Bla', 'TITEL BLA', 1200, 800)
    Canvas.cd()  # activate
    Canvas.SetLogy()
    Canvas.SetLeftMargin(0.13)
    Canvas.SetRightMargin(0.04)
    Canvas.SetTopMargin(0.03)
    Canvas.SetBottomMargin(0.14)
    hist_signal.SetFillColorAlpha(ROOT.kBlue, 0.2)
    hist_signal.SetFillStyle(1001)
    hist_signal.SetTitleSize(0.1)
    hist_signal.GetXaxis().SetLabelSize(0.04)
    hist_signal.GetYaxis().SetLabelSize(0.04)
    hist_signal.GetXaxis().SetTitleSize(0.06)
    hist_signal.GetYaxis().SetTitleSize(0.06)
    hist_signal.GetXaxis().SetLabelSize(0.04)
    hist_signal.GetYaxis().SetLabelSize(0.04)
    hist_signal.GetXaxis().SetTitleSize(0.05)
    hist_signal.GetYaxis().SetTitleSize(0.05)
    hist_signal.GetXaxis().SetTitleOffset(0.95)
    hist_signal.GetYaxis().SetTitleOffset(1.1)
    hist_signal.GetXaxis().SetTitleOffset(1.15)
    hist_signal.GetYaxis().SetLimits(0, Ymax)
    hist_signal.SetLineColor(ROOT.kBlue)
    hist_background.SetFillColorAlpha(ROOT.kRed, 1.0)
    hist_background.SetFillStyle(3005)
    hist_background.GetYaxis().SetLimits(0, Ymax)
    hist_background.SetLineColor(ROOT.kRed)

    catName = category
    if category == 'MaximumPstar':
        catName = 'MaximumP*'

    hist_signal.SetTitle('; (#it{qp})^{' + catName + '} ; Events')
    # hist_signal.SetMinimum(0)
    hist_signal.SetMaximum(Ymax)
    # hist_background.SetMinimum(0)
    hist_background.SetMaximum(Ymax)

    hist_signal.Draw('hist')
    hist_background.Draw('hist same')

    l0 = ROOT.TLegend(0.13, 0.65, 0.33, 0.97)
    l0.SetFillColorAlpha(ROOT.kWhite, 0)
    l0.AddEntry(hist_signal, ' #varepsilon_{eff}(B^{0}) = ' + '{:.2f}'.format(tot_eff_effB0 * 100) + "%")
    l0.AddEntry(hist_background, ' #varepsilon_{eff}(#bar{B}^{0}) = ' + '{:.2f}'.format(tot_eff_effB0bar * 100) + "%")
    l0.AddEntry("", "#bf{#varepsilon_{eff} = " + '{:.2f}'.format(effAverage * 100) + '%}')
    l0.AddEntry("", '#Delta#varepsilon_{eff} = ' + '{:^5.2f}'.format(effDiff * 100) + '%')
    l0.SetBorderSize(0)
    l0.SetTextSize(0.045)
    l0.Draw()

    l1 = ROOT.TLegend(0.85, 0.7, 0.96, 0.97)
    l1.SetFillColorAlpha(ROOT.kWhite, 0.35)
    l1.AddEntry(hist_signal, 'B^{0}_{MC}')
    l1.AddEntry(hist_background, '#bar{B}^{0}_{MC}')
    l1.SetTextSize(0.045)
    l1.Draw()

    Canvas.Update()
    with Quiet(ROOT.kError):
        Canvas.SaveAs(workingDirectory + '/' + 'test6_CPVFTqp_' + category + '_both.pdf')

    # Validation Plot 4
    hist_both.GetXaxis().SetLabelSize(0.04)
    hist_both.GetYaxis().SetLabelSize(0.04)
    hist_both.GetYaxis().SetTitleOffset(0.7)
    hist_both.GetXaxis().SetTitleOffset(0.7)
    hist_both.GetXaxis().SetTitleSize(0.06)
    hist_both.GetYaxis().SetTitleSize(0.06)

    hist_both.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats, logy'))
    hist_both.GetListOfFunctions().Add(ROOT.TNamed('Description', 'Output of the flavor tagger category ' + catName))
    hist_both.GetListOfFunctions().Add(
        ROOT.TNamed('Check', 'Shape should not change drastically. E.g. Warning if there is only a peak at 0.'))
    hist_both.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'abudinen@mpp.mpg.de'))

    hist_both.SetTitle(
        'Flavor tagger output of the category ' +
        catName +
        '; #it{qp}_{' +
        catName +
        '} ; Events  (Total = ' +
        '{:<1}'.format(
            "%.0f" %
            hist_both.GetEntries()) +
        ')')
    # hist_both.SetStats(False)
    hist_both.Write()

    Canvas.Clear()

outputNtuple.Fill(array('f', efficienciesForNtuple))
outputNtuple.Write()
outputFile.Close()

print('*                                                                                                                 *')
print('*******************************************************************************************************************')

if not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging', True):
    B2FATAL('flavorTaggerEfficiency: THE "FlavorTagging" DIRECTORY COULD NOT BE FOUND IN THE WORKING DIRECTORY.')
else:
    shutil.rmtree(workingDirectory + '/FlavorTagging')
    B2INFO('flavorTaggerEfficiency: THE "FlavorTagging" DIRECTORY WAS FOUND IN THE WORKING DIRECTORY AND DELETED.')
