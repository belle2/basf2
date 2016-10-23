#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Author: Moritz Gelb & Fernando Abudinen  *
# * Script for autmating reporting           *
# * flavor tagging                           *
#                                            *
# ********************************************

from basf2 import *
from flavorTagger import *
import ROOT
import pdg

import math
import re
import subprocess
import copy
from string import Template
from modularAnalysis import *
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False
from array import array
import IPython

ROOT.gROOT.SetBatch(True)

if len(sys.argv) != 3:
    sys.exit("Must provide 2 arguments: [input_sim_file] or ['input_sim_file*'] with wildcards and [treeName]"
             )
workingFile = sys.argv[1]
workingFiles = glob.glob(str(workingFile))
treeName = str(sys.argv[2])

if len(workingFiles) < 1:
    sys.exit("No file name or file names " + str(workingFile) + " found.")

# workingDirectory = \
    # Belle2.FileSystem.findFile('/analysis/examples/tutorials')

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
    'FastPion',
    'Lambda']


methods = []


class Quiet:

    def __init__(self, level=ROOT.kInfo + 1):
        self.level = level

    def __enter__(self):
        self.oldlevel = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = self.level

    def __exit__(self, type, value, traceback):
        ROOT.gErrorIgnoreLevel = self.oldlevel

# root-file
# rootfile = ROOT.TFile(workingFile, 'READ')
# rootfile.cd()
# tree = rootfile.Get(treeName)
# tree = rootfile.Get('Bd_JpsiKS_tuple')
# rootfile.Close()
tree = ROOT.TChain(treeName)

mcstatus = array('d', [-511.5, 0.0, 511.5])
ROOT.TH1.SetDefaultSumw2()

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
    catBranch = 'B0_qr' + cat
    if catBranch in totalBranches:
        usedCategories.append(cat)

if len(usedCategories) > 1:
    WhichCategories(usedCategories)

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
    histo_belleplotB0 = ROOT.TH1F('BellePlot_B0',
                                  'BellePlot for true B0 (binning 50)', 50,
                                  -1.0, 1.0)
    # histogram containing the belle paper plot (qr-tagger output for true B0bar)
    histo_belleplotB0bar = ROOT.TH1F('BellePlot_B0Bar',
                                     'BellePlot for true B0Bar (binning 50)',
                                     50, -1.0, 1.0)
    # calibration plot for B0. If we get a linaer line our MC is fine, than the assumption r ~ 1- 2w is reasonable
    # expectation is, that for B0 calibration plot:  qr=0  half B0 and half B0bar, qr = 1 only B0 and qr = -1
    # no B0. Inverse for B0bar calibration plot
    histo_calib_B0 = ROOT.TH1F('Calibration_B0', 'CalibrationPlot for true B0', 100, -1.0, 1.0)
    # calibration plot for B0bar calibration plot
    histo_calib_B0bar = ROOT.TH1F('Calibration_B0Bar',
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

    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_B0', 'B0_qrMC == 1')
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_B0Bar', 'B0_qrMC == -1')
    tree.Draw('B0_' + method + '_qrCombined>>BellePlot_NoCut', 'abs(B0_qrMC) == 1')

    tree.Draw('B0_' + method + '_qrCombined>>Calibration_B0', 'B0_qrMC == 1')
    tree.Draw('B0_' + method + '_qrCombined>>Calibration_B0Bar', 'B0_qrMC == -1')

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
    tree.Project('Average_rB0', 'abs(B0_' + method + '_qrCombined)', 'abs(B0_' + method + '_qrCombined)*(B0_qrMC>0)')
    tree.Project('Average_rB0bar', 'abs(B0_' + method + '_qrCombined)', 'abs(B0_' + method + '_qrCombined)*(B0_qrMC<0)')

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

    print('*                 -->  DETERMINATION BASED ON MONTE CARLO                                          *')
    print('*                                                                                                  *')
    print('* ------------------------------------------------------------------------------------------------ *')
    print('*   r-interval        <r>        Efficiency       Delta_Efficiency        w         Delta_w        *')
    print('* ------------------------------------------------------------------------------------------------ *')
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
        # intervallEff[i] = event_fractionTotal[i] * rvalueB0Average[i] * rvalueB0Average[i]
        print('* ' + '{:.3f}'.format(r_subsample[i - 1]) + ' - ' + '{:.3f}'.format(r_subsample[i]) + '      ' +
              '{:.3f}'.format(rvalueB0Average[i]) + '          ' +
              '{:.4f}'.format(event_fractionTotal[i]) + '           ' +
              '{: .4f}'.format(event_fractionDiff[i]) + '           ' +
              '{:.4f}'.format(wvalue[i]) + '       ' +
              '{: .4f}'.format(wvalueDiff[i]) + '        *')
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

    # not that imortant
    print('* ---------------------------------                                                                *')
    print('* Efficiency Determination - easiest way                                                           *')
    print('* ---------------------------------                                                                *')
    total_entries_B0 = histo_belleplotB0.GetEntries()
    total_entries_B0Bar = histo_belleplotB0bar.GetEntries()
    total_entries_B0_wrong = histo_m1.GetEntries()
    total_entries_B0Bar_wrong = histo_m2.GetEntries()
    total_entries = total_entries_B0 + total_entries_B0Bar
    total_entries_wrong = total_entries_B0_wrong + total_entries_B0Bar_wrong

    wrong_tag_fraction_B0 = total_entries_B0_wrong / total_entries_B0
    wrong_tag_fraction_B0Bar = total_entries_B0Bar_wrong / total_entries_B0Bar
    wrong_tag_fraction = total_entries_wrong / total_entries
    right_tag_fraction_B0 = 1 - 2 * wrong_tag_fraction_B0
    right_tag_fraction_B0Bar = 1 - 2 * wrong_tag_fraction_B0Bar
    right_tag_fraction = 1 - 2 * wrong_tag_fraction
    wrong_eff_B0 = right_tag_fraction_B0 * right_tag_fraction_B0
    wrong_eff_B0Bar = right_tag_fraction_B0Bar * right_tag_fraction_B0Bar
    wrong_eff = right_tag_fraction * right_tag_fraction

    print('*     wrong_tag_fraction for all:   ' +
          '{:.3f}'.format(wrong_tag_fraction * 100) +
          ' %                                                       *')
    print('*     right_tag_fraction for all:   ' +
          '{:.3f}'.format(right_tag_fraction * 100) +
          ' %                                                       *')
    print('*     wrong calculated eff all:     ' + '{:.3f}'.format(wrong_eff * 100) +
          ' %                                                       *')
    print('*                                                                                                  *')

    # write out the histograms
    # histo_avr_r.Write('', ROOT.TObject.kOverwrite)
    # histo_entries_per_bin.Write('', ROOT.TObject.kOverwrite)

    # histo_Cnet_output_B0.Write('', ROOT.TObject.kOverwrite)
    # histo_Cnet_output_B0bar.Write('', ROOT.TObject.kOverwrite)
    # histo_belleplotB0.Write('', ROOT.TObject.kOverwrite)
    # histo_belleplotB0bar.Write('', ROOT.TObject.kOverwrite)
    # histo_calib_B0.Write('', ROOT.TObject.kOverwrite)
    # histo_calib_B0bar.Write('', ROOT.TObject.kOverwrite)

    maxB0 = histo_belleplotB0.GetBinContent(histo_belleplotB0.GetMaximumBin())
    maxB0bar = histo_belleplotB0bar.GetBinContent(histo_belleplotB0bar.GetMaximumBin())

    Ymax = max(maxB0, maxB0bar)
    Ymax = Ymax + Ymax / 12

    if YmaxForQrPlot < Ymax:
        YmaxForQrPlot = Ymax

    # produce a pdf
    ROOT.gStyle.SetOptStat(0)
    Canvas1 = ROOT.TCanvas('Bla', 'Final Output', 1200, 800)
    Canvas1.cd()  # activate
    histo_belleplotB0.SetFillColorAlpha(ROOT.kBlue, 0.2)
    histo_belleplotB0.SetFillStyle(1001)
    histo_belleplotB0.GetYaxis().SetLabelSize(0.03)
    histo_belleplotB0.GetYaxis().SetLimits(0, YmaxForQrPlot)
    histo_belleplotB0.GetYaxis().SetTitleOffset(1.2)
    histo_belleplotB0.SetLineColor(ROOT.kBlue)
    histo_belleplotB0bar.SetFillColorAlpha(ROOT.kRed, 1.0)
    histo_belleplotB0bar.SetFillStyle(3005)
    histo_belleplotB0bar.SetLineColor(ROOT.kRed)
    # SetLabelSize etc SetTitle

    histo_belleplotB0.SetTitle('Final Flavor Tagger Output; #it{qr}-output ; Events'
                               )
    histo_belleplotB0.SetMinimum(0)
    histo_belleplotB0.SetMaximum(YmaxForQrPlot)
    histo_belleplotB0.Draw('hist')
    histo_belleplotB0bar.Draw('hist same')

    leg = ROOT.TLegend(0.75, 0.8, 0.9, 0.9)
    leg.AddEntry(histo_belleplotB0, 'true B0')
    leg.AddEntry(histo_belleplotB0bar, 'true B0bar')
    leg.Draw()

    Canvas1.Update()
    # IPython.embed()
    with Quiet(ROOT.kError):
        Canvas1.SaveAs(workingDirectory + '/' + 'PIC_Belleplot_both' + method + '.pdf')

    # produce the nice calibration plot
    Canvas2 = ROOT.TCanvas('Bla2', 'Calibration plot for true B0', 1200, 800)
    Canvas2.cd()  # activate
    histo_calib_B0.SetFillColorAlpha(ROOT.kBlue, 0.2)
    histo_calib_B0.SetFillStyle(1001)
    histo_calib_B0.GetYaxis().SetTitleOffset(1.2)
    histo_calib_B0.SetLineColor(ROOT.kBlue)

    histo_calib_B0.SetTitle('Calibration For True B0; #it{qr}-output ; Calibration '
                            )
    histo_calib_B0.Draw('hist')
    diag.Draw('SAME')
    Canvas2.Update()
    with Quiet(ROOT.kError):
        Canvas2.SaveAs(workingDirectory + '/' + 'PIC_Calibration_B0' + method + '.pdf')

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
    Canvas1.Destructor()
    Canvas2.Destructor()

    print('\\begin{tabular}{| l | r  r | r  r | r  r |}\n\\hline')
    print(r'$r$- Interval & $\varepsilon_i(\%)$ & $\Delta \varepsilon_i(\%) $ & $w_i(\%) $ ' +
          r' & $\Delta w_i(\%) $& $\varepsilon_{\text{eff}, i}(\%) $& $\Delta \varepsilon_{\text{eff}, i}(\%) $\\ \hline\hline')
    for i in range(1, r_size):
        print('$ ' + '{:.3f}'.format(r_subsample[i - 1]) + ' - ' + '{:.3f}'.format(r_subsample[i]) + '$ & $'
              '{: 6.1f}'.format(event_fractionTotal[i] * 100) + '$ & $' +
              '{: 6.1f}'.format(event_fractionDiff[i] * 100) + '$ & $' +
              '{: 6.1f}'.format(wvalue[i] * 100) + '$ & $' +
              '{: 6.1f}'.format(wvalueDiff[i] * 100) + '$ & $' +
              '{: 6.1f}'.format(iEffEfficiency[i] * 100) + '$ & $' +
              '{: 6.1f}'.format(iDeltaEffEfficiency[i] * 100) + r'$ \\ ')
    print('\hline\hline')
    print(r'\multicolumn{1}{|r|}{Total} &  \multicolumn{4}{r|}{ $\varepsilon_\text{eff} = ' +
          r'\sum_i \varepsilon_i \cdot \langle 1-2w_i\rangle^2 = ' +
          '{: 6.1f}'.format(average_eff * 100) + r' \%$ }')
    print(r'& \multicolumn{2}{r|}{ $\Delta \varepsilon_\text{eff} = ' +
          '{: 6.1f}'.format(diff_eff * 100) + r' \%$ }' + r' \\')
    print('\\hline\n\\end{tabular}')


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
categoriesPerformance = []
for (particleList, category, combinerVariable) in eventLevelParticleLists:
    # histogram of input variable (only signal) - not yet a probability! It's a classifier plot!
    hist_signal = ROOT.TH1F('Signal_' + category, 'Input Signal (B0)' +
                            category + ' (binning 50)', 50, -1.0, 1.0)
    # histogram of input variable (only background) - not yet a probability! It's a classifier plot!
    hist_background = ROOT.TH1F('Background_' + category, 'Input Background (B0bar)' +
                                category + ' (binning 50)', 50, -1.0, 1.0)

    # per definiton that input is not comparable to the network output, this has to be transformed.
    # probability output from 0 to 1 (corresponds to net output probability) -> calculation below
    hist_probB0 = ROOT.TH1F('ProbabilityB0_' + category,
                            'Transformed to probability (B0) (' + category + ')',
                            50, 0.0, 1.0)
    hist_probB0bar = ROOT.TH1F('ProbabilityB0bar_' + category,
                               'Transformed to probability (B0bar) (' + category + ')',
                               50, 0.0, 1.0)
    # qr output from -1 to 1 -> transformation below
    hist_qrB0 = ROOT.TH1F('QRB0_' + category, 'Transformed to qr (B0)(' +
                          category + ')', 50, -1.0, 1.0)
    hist_qrB0bar = ROOT.TH1F('QRB0bar_' + category, 'Transformed to qr (B0bar) (' +
                             category + ')', 50, -1.0, 1.0)
    # histogram for abs(qr), i.e. this histogram contains the r-values -> transformation below
    # also used to get the number of entries, sorted into 6 bins
    hist_absqrB0 = ROOT.TH1F('AbsQRB0_' + category, 'Abs(qr)(B0) (' + category + ')', 6, r_subsample)
    hist_absqrB0bar = ROOT.TH1F('AbsQRB0bar_' + category, 'Abs(qr) (B0bar) (' + category + ')', 6, r_subsample)
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
    tree.Draw('B0_qr' + category + '>>All_' + category, 'B0_qrMC!=0')
    hist_calib_B0 = ROOT.TH1F('Calib_B0_' + category, 'Calibration Plot for true B0' +
                              category + ' (binning 50)', 50, 0.0, 1.0)
    tree.Draw('B0_qr' + category + '>>Calib_B0_' + category, 'B0_qrMC == 1.0')
    hist_calib_B0.Divide(hist_all)

    # fill signal
    tree.Draw('B0_qr' + category + '>>Signal_' + category, 'B0_qrMC == 1.0')
    # fill background
    tree.Draw('B0_qr' + category + '>>Background_' + category, 'B0_qrMC == -1.0'
              )

    # ****** produce the input plots from combiner level ******

    maxSignal = hist_signal.GetBinContent(hist_signal.GetMaximumBin())
    maxBackground = hist_background.GetBinContent(hist_background.GetMaximumBin())

    Ymax = max(maxSignal, maxBackground)
    Ymax = Ymax + Ymax / 12

    ROOT.gStyle.SetOptStat(0)
    Canvas = ROOT.TCanvas('Bla', 'TITEL BLA', 1200, 800)
    Canvas.cd()  # activate
    hist_signal.SetFillColorAlpha(ROOT.kBlue, 0.2)
    hist_signal.SetFillStyle(1001)
    hist_signal.SetTitleSize(0.1)
    hist_signal.GetXaxis().SetLabelSize(0.04)
    hist_signal.GetYaxis().SetLabelSize(0.04)
    hist_signal.GetXaxis().SetTitleSize(0.05)
    hist_signal.GetYaxis().SetTitleSize(0.05)
    hist_signal.GetXaxis().SetTitleOffset(0.95)
    hist_signal.GetYaxis().SetTitleOffset(1.1)
    hist_signal.GetYaxis().SetLimits(0, Ymax)
    hist_signal.SetLineColor(ROOT.kBlue)
    hist_background.SetFillColorAlpha(ROOT.kRed, 1.0)
    hist_background.SetFillStyle(3005)
    hist_background.GetYaxis().SetLimits(0, Ymax)
    hist_background.SetLineColor(ROOT.kRed)

    hist_signal.SetTitle(category + ' category; #it{qr}-Output ; Events')
    hist_signal.SetMinimum(0)
    hist_signal.SetMaximum(Ymax)
    hist_background.SetMinimum(0)
    hist_background.SetMaximum(Ymax)

    hist_signal.Draw('hist')
    hist_background.Draw('hist same')

    if category == 'MaximumPstar':
        l = ROOT.TLegend(0.4, 0.75, 0.6, 0.9)
    else:
        l = ROOT.TLegend(0.6, 0.75, 0.8, 0.9)
    l.AddEntry(hist_signal, 'true B0')
    l.AddEntry(hist_background, 'true B0bar')
    l.SetTextSize(0.05)
    l.Draw()

    Canvas.Update()
    with Quiet(ROOT.kError):
        Canvas.SaveAs(workingDirectory + '/' + 'PIC_' + category + '_Input_Combiner.pdf')

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

        # filling histogram with qr from -1 to 1
        hist_qrB0.Fill(dilutionB02[i], signal[i])
        hist_qrB0bar.Fill(dilutionB0bar2[i], back[i])

        # filling histogram with abs(qr), i.e. this histogram contains the r-values (not qr)
        hist_absqrB0.Fill(abs(dilutionB02[i]), signal[i])
        hist_absqrB0bar.Fill(abs(dilutionB0bar2[i]), back[i])
        # filling histogram with abs(qr) special weighted - needed for average r calculation
        hist_aver_rB0.Fill(abs(dilutionB02[i]), abs(dilutionB02[i]) * signal[i])
        hist_aver_rB0bar.Fill(abs(dilutionB0bar2[i]), abs(dilutionB0bar2[i]) * back[i])

    # hist_aver_rB0bar contains now the average r-value
    hist_aver_rB0.Divide(hist_absqrB0)
    hist_aver_rB0bar.Divide(hist_absqrB0bar)
    # now calculating the efficiency

    # calculating number of events
    tot_entriesB0 = 0
    tot_entriesB0bar = 0
    for i in range(1, r_size):
        tot_entriesB0 = tot_entriesB0 + hist_absqrB0.GetBinContent(i)
        tot_entriesB0bar = tot_entriesB0bar + hist_absqrB0bar.GetBinContent(i)
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
        entriesB0[i] = hist_absqrB0.GetBinContent(i)
        entriesB0bar[i] = hist_absqrB0bar.GetBinContent(i)
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

    # hist_signal.Write('', ROOT.TObject.kOverwrite)
    # hist_background.Write('', ROOT.TObject.kOverwrite)
    # hist_probB0.Write('', ROOT.TObject.kOverwrite)
    # hist_probB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_qrB0.Write('', ROOT.TObject.kOverwrite)
    # hist_qrB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_absqrB0.Write('', ROOT.TObject.kOverwrite)
    # hist_absqrB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_aver_rB0.Write('', ROOT.TObject.kOverwrite)
    # hist_aver_rB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_all.Write('', ROOT.TObject.kOverwrite)
    # hist_calib_B0.Write('', ROOT.TObject.kOverwrite)
    categoriesPerformance.append((category, effAverage, effDiff))
    Canvas.Destructor()
# if average_eff != 0:
    # print '*    -------------------------------------------------------------------------'
    # print '*    ' + '{: > 8.2f}'.format(average_eff * 100) + ' %' \
    # + '{:>85}'.format('TOTAL' + '                      *')

print('*                                                                                                                 *')
print('*******************************************************************************************************************')
print('\\begin{tabular}{| l | r  r |}\n\hline')
print(r'Categories & $\varepsilon_\text{eff}(\%) $& $\Delta \varepsilon_\text{eff}(\%) $\\ \hline\hline')
for (category, effAverage, effDiff) in categoriesPerformance:
    print(
        '{:<23}'.format(category) +
        ' & $' +
        '{: 6.2f}'.format(
            effAverage *
            100) +
        ' $ & $' +
        '{: 6.2f}'.format(
            effDiff *
            100) +
        r' $ \\')
print("\hline\n\\end{tabular}")
B2INFO('qr Output Histograms in pdf format saved at: ' + workingDirectory)
