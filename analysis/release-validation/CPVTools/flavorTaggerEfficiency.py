#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# **************  Flavor Tagging   **************
# *                                             *
# * This script calculates the paramters charac-*
# * terizing the performance of the flavor tag- *
# * ger. It produces as well plots for the qr   *
# * distribution of the combiners and the dis-  *
# * tributions of the combiner input variables. *
# *                                             *
# ***********************************************

import ROOT
from basf2 import B2INFO
import flavorTagger as ft
from defaultEvaluationParameters import categories, Quiet, r_subsample, r_size
from array import array
import pickle
import math
import glob
import sys

ROOT.gROOT.SetBatch(True)

if len(sys.argv) != 3:
    sys.exit("Must provide 2 arguments: [input_sim_file] or ['input_sim_file*'] with wildcards and [treeName]"
             )
workingFile = sys.argv[1]
workingFiles = glob.glob(str(workingFile))
treeName = str(sys.argv[2])

if len(workingFiles) < 1:
    sys.exit("No file name or file names " + str(workingFile) + " found.")


workingDirectory = '.'

#
# *****************************************
# DETERMINATION OF TOTAL EFFECTIVE EFFIENCY
# *****************************************
#

# working directory
# needs the B0bar_final.root-file
# treeName = 'B0tree'


methods = []

tree = ROOT.TChain(treeName)

mcstatus = array('d', [-511.5, 0.0, 511.5])
ROOT.TH1.SetDefaultSumw2()

for iFile in workingFiles:
    tree.AddFile(iFile)

totalBranches = []
for branch in tree.GetListOfBranches():
    totalBranches.append(branch.GetName())

if 'FBDT_qrCombined' in totalBranches:
    methods.append("FBDT")

if 'FANN_qrCombined' in totalBranches:
    methods.append("FANN")

if 'DNN_qrCombined' in totalBranches:
    methods.append("DNN")

usedCategories = []
for cat in categories:
    catBranch = 'qp' + cat
    if catBranch in totalBranches:
        usedCategories.append(cat)

if len(usedCategories) > 1:
    ft.WhichCategories(usedCategories)

YmaxForQrPlot = 0

total_notTagged = 0

for method in methods:
    # Get erro with GetBinError(), set erro with SetBinError()
    # histogram contains the average r in each of 7 bins -> calculation see below
    histo_avr_r = ROOT.TH1F('Average_r', 'Average r in each of the bins (B0 and B0bar)', int(r_size - 2),
                            r_subsample)
    histo_avr_rB0 = ROOT.TH1F('Average_rB0', 'Average r in each of the bins (B0)', int(r_size - 2),
                              r_subsample)
    histo_avr_rB0bar = ROOT.TH1F('Average_rB0bar', 'Average r in each of the bins (B0bar)', int(r_size - 2),
                                 r_subsample)

    # histogram containing the wrong tags calculated by counting Wrong tags
    histo_mc_NwB0 = ROOT.TH1F('mc_NwB0', 'Average r in each of the bins (B0)', int(r_size - 2),
                              r_subsample)
    histo_mc_NwB0bar = ROOT.TH1F('mc_NwB0bar', 'Average r in each of the bins (B0bar)', int(r_size - 2),
                                 r_subsample)

    # histogram contains the mean squared of r in each of 7 bins -> calculation see below
    histo_ms_r = ROOT.TH1F('MS_r', 'Mean squared average of r in each of the bins (B0 and B0bar)', int(r_size - 2),
                           r_subsample)
    histo_ms_rB0 = ROOT.TH1F('MS_rB0', 'Mean squared average of r in each of the bins (B0)', int(r_size - 2),
                             r_subsample)
    histo_ms_rB0bar = ROOT.TH1F('MS_rB0bar', 'Mean squared average of r in each of the bins (B0bar)', int(r_size - 2),
                                r_subsample)

    # histogram with number of entries in for each bin
    histo_entries_per_bin = ROOT.TH1F(
        'entries_per_bin',
        'Events binned in r_subsample according to their r-value for B0 and B0bar prob',
        int(r_size - 2),
        r_subsample)
    histo_entries_per_binB0 = ROOT.TH1F('entries_per_binB0', 'Events binned in r_subsample according '
                                        'to their r-value for B0 prob', int(r_size - 2), r_subsample)
    histo_entries_per_binB0bar = ROOT.TH1F('entries_per_binB0bar',
                                           'Events binned in r_subsample according to their r-value '
                                           'for B0bar prob', int(r_size - 2), r_subsample)
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

    histo_notTaggedEvents = ROOT.TH1F('notTaggedEvents',
                                      'Histogram for not tagged events',
                                      1, -3.0, -1.0)

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
    histo_m0 = ROOT.TH1F('BellePlot_m0',
                         'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m1 = ROOT.TH1F('BellePlot_m1',
                         'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m2 = ROOT.TH1F('BellePlot_m2',
                         'BellePlot_m for true B0Bar (binning 50)', 50, -1.0,
                         1.0)

    # filling the histograms

    tree.Draw(method + '_qrCombined>>BellePlot_B0', 'qrMC == 1')
    tree.Draw(method + '_qrCombined>>BellePlot_B0Bar', 'qrMC == -1')
    tree.Draw(method + '_qrCombined>>BellePlot_NoCut', 'abs(qrMC) == 1')

    tree.Draw(method + '_qrCombined>>Calibration_B0', 'qrMC == 1')
    tree.Draw(method + '_qrCombined>>Calibration_B0Bar', 'qrMC == -1')

    tree.Draw(method + '_qrCombined>>notTaggedEvents',
              'abs(qrMC) == 0 && isSignal == 1 && ' +
              method + '_qrCombined < -1')

    # filling histograms wrong efficiency calculation
    tree.Draw(method + '_qrCombined>>BellePlot_m0',
              'qrMC == 1 && ' + method + '_qrCombined>0')
    tree.Draw(method + '_qrCombined>>BellePlot_m1',
              'qrMC == 1 && ' + method + '_qrCombined<0')
    tree.Draw(method + '_qrCombined>>BellePlot_m2',
              'qrMC == -1 && ' + method + '_qrCombined>0 ')

    # filling with abs(qr) in one of 6 bins with its weight
    # separate calculation for B0 and B0bar

    tree.Project('Average_r', 'abs(' + method + '_qrCombined)', 'abs(' + method + '_qrCombined)*(abs(qrMC) == 1)')
    tree.Project('Average_rB0', 'abs(' + method + '_qrCombined)', 'abs(' + method + '_qrCombined)*(qrMC==1)')
    tree.Project('Average_rB0bar', 'abs(' + method + '_qrCombined)', 'abs(' + method + '_qrCombined)*(qrMC==-1)')

    tree.Project('MS_r', 'abs(' + method + '_qrCombined)', '(' + method +
                 '_qrCombined*' + method + '_qrCombined)*(abs(qrMC) == 1)')
    tree.Project('MS_rB0', 'abs(' + method + '_qrCombined)',
                 '(' + method + '_qrCombined*' + method + '_qrCombined)*(qrMC==1)')
    tree.Project('MS_rB0bar', 'abs(' + method + '_qrCombined)',
                 '(' + method + '_qrCombined*' + method + '_qrCombined)*(qrMC==-1)')

    # filling with abs(qr) in one of 6 bins
    tree.Project('entries_per_bin', 'abs(' + method + '_qrCombined)', 'abs(qrMC) == 1')
    tree.Project('entries_per_binB0', 'abs(' + method + '_qrCombined)', 'qrMC == 1')
    tree.Project('entries_per_binB0bar', 'abs(' + method + '_qrCombined)', 'qrMC == -1')

    # filling histograms with number of wrong tags per r-bin
    tree.Project('mc_NwB0', 'abs(' + method + '_qrCombined)', ' ' + method + '_qrCombined*qrMC < 0 && qrMC == 1')
    tree.Project('mc_NwB0bar', 'abs(' + method + '_qrCombined)', ' ' + method + '_qrCombined*qrMC < 0 && qrMC == -1')

    # producing the average r histograms
    histo_avr_r.Divide(histo_entries_per_bin)
    histo_avr_rB0.Divide(histo_entries_per_binB0)
    histo_avr_rB0bar.Divide(histo_entries_per_binB0bar)

    histo_ms_r.Divide(histo_entries_per_bin)
    histo_ms_rB0.Divide(histo_entries_per_binB0)
    histo_ms_rB0bar.Divide(histo_entries_per_binB0bar)

    # producing the calibration plots
    # Errors ok
    histo_calib_B0.Divide(hallo12)
    histo_calib_B0bar.Divide(hallo12)

    # Fit for calibration plot
    print(' ')
    print('****************************** CALIBRATION CHECK FOR COMBINER USING ' +
          method + ' ***************************************')
    print(' ')
    print('Fit ploynomial of first order to the calibration plot. Expected value ~0.5')
    print(' ')
    histo_calib_B0.Fit(diag, 'TEST')
    print('       ')
    print('****************************** MEASURED EFFECTIVE EFFICIENCY FOR COMBINER USING ' +
          method + ' ***********************************')
    print('*                                                                               ' +
          '                                       *')
    # get total number of entries
    total_tagged = histo_entries_per_bin.GetEntries()
    total_tagged_B0 = histo_entries_per_binB0.GetEntries()
    total_tagged_B0bar = histo_entries_per_binB0bar.GetEntries()
    total_notTagged = histo_notTaggedEvents.GetEntries()
    total_entries = (total_tagged + total_notTagged)
    # To a good approximation we assume that half of the not tagged B mesons were B0 (B0bar)
    total_entriesB0 = (total_tagged_B0 + total_notTagged / 2)
    total_entriesB0bar = (total_tagged_B0bar + total_notTagged / 2)

    tagging_eff = total_tagged / (total_tagged + total_notTagged)
    DeltaTagging_eff = math.sqrt(total_tagged * total_notTagged**2 + total_notTagged * total_tagged**2) / (total_entries**2)
    tot_eff_effB0 = 0
    tot_eff_effB0bar = 0
    average_eff_eff = 0
    uncertainty_eff_effB0 = 0
    uncertainty_eff_effB0bar = 0
    uncertainty_eff_effAverage = 0
    diff_eff_Uncertainty = 0
    event_fractionB0 = array('f', [0] * r_size)
    event_fractionB0bar = array('f', [0] * r_size)
    event_fractionTotal = array('f', [0] * r_size)
    event_fractionTotalUncertainty = array('f', [0] * r_size)
    eventsInBin_B0 = array('f', [0] * r_size)
    eventsInBin_B0bar = array('f', [0] * r_size)
    eventsInBin_Total = array('f', [0] * r_size)
    event_fractionDiff = array('f', [0] * r_size)
    event_fractionDiffUncertainty = array('f', [0] * r_size)
    rvalueB0 = array('f', [0] * r_size)
    rvalueB0bar = array('f', [0] * r_size)
    rvalueB0Average = array('f', [0] * r_size)
    rvalueStdB0 = array('f', [0] * r_size)
    rvalueStdB0bar = array('f', [0] * r_size)
    rvalueStdB0Average = array('f', [0] * r_size)
    wvalue = array('f', [0] * r_size)
    wvalueUncertainty = array('f', [0] * r_size)
    wvalueB0 = array('f', [0] * r_size)
    wvalueB0bar = array('f', [0] * r_size)
    wvalueB0Uncertainty = array('f', [0] * r_size)
    wvalueB0barUncertainty = array('f', [0] * r_size)
    wvalueDiff = array('f', [0] * r_size)
    wvalueDiffUncertainty = array('f', [0] * r_size)
    entries = array('f', [0] * r_size)
    entriesB0 = array('f', [0] * r_size)
    entriesB0bar = array('f', [0] * r_size)
    iEffEfficiency = array('f', [0] * r_size)
    iEffEfficiencyUncertainty = array('f', [0] * r_size)
    iEffEfficiencyB0Uncertainty = array('f', [0] * r_size)
    iEffEfficiencyB0barUncertainty = array('f', [0] * r_size)
    iEffEfficiencyB0UncertaintyFromOutput = array('f', [0] * r_size)
    iEffEfficiencyB0barUncertaintyFromOutput = array('f', [0] * r_size)

    iDeltaEffEfficiency = array('f', [0] * r_size)
    iDeltaEffEfficiencyUncertainty = array('f', [0] * r_size)
    muParam = array('f', [0] * r_size)
    muParamUncertainty = array('f', [0] * r_size)
    # intervallEff = array('f', [0] * r_size)

    print('*                 -->  DETERMINATION BASED ON MONTE CARLO           ' +
          '                                                   *')
    print('*                                                                   ' +
          '                                                   *')
    print('*  Note: mu = Delta_Effcy/(2*Efficiency). Needed for CP analysis ' +
          'together with w and  Delta_w                          *')
    print('*                                                                   ' +
          '                                                   *')
    print('* ------------------------------------------------------------------' +
          '-------------------------------------------------- *')
    print('*   r-interval          <r>        Efficiency   Delta_Effcy      ' +
          '     mu                 w               Delta_w       *')
    print('* ------------------------------------------------------------------' +
          '-------------------------------------------------- *')
    performance = []
    for i in range(1, r_size):
        # get the average r-value
        entries[i] = histo_entries_per_bin.GetBinContent(i)
        entriesB0[i] = histo_entries_per_binB0.GetBinContent(i)
        entriesB0bar[i] = histo_entries_per_binB0bar.GetBinContent(i)
        # fraction of events/all events

        event_fractionB0[i] = entriesB0[i] / total_entriesB0
        event_fractionB0bar[i] = entriesB0bar[i] / total_entriesB0bar

        # event_fractionTotal[i] = (entriesB0[i] + entriesB0bar[i]) / total_entries
        # event_fractionDiff[i] = (entriesB0[i] - entriesB0bar[i]) / total_entries

        event_fractionTotal[i] = (event_fractionB0[i] + event_fractionB0bar[i]) / 2
        event_fractionDiff[i] = event_fractionB0[i] - event_fractionB0bar[i]

        event_fractionDiffUncertainty[i] = math.sqrt(entriesB0[i] *
                                                     (total_entriesB0 -
                                                      entriesB0[i]) /
                                                     total_entriesB0**3 +
                                                     entriesB0bar[i] *
                                                     (total_entriesB0bar -
                                                      entriesB0bar[i]) /
                                                     total_entriesB0bar**3)

        event_fractionTotalUncertainty[i] = event_fractionDiffUncertainty[i] / 2

        rvalueB0[i] = histo_avr_rB0.GetBinContent(i)
        rvalueB0bar[i] = histo_avr_rB0bar.GetBinContent(i)
        rvalueB0Average[i] = histo_avr_r.GetBinContent(i)  # (rvalueB0[i] + rvalueB0bar[i]) / 2
        rvalueStdB0[i] = math.sqrt(histo_ms_rB0.GetBinContent(
            i) - (histo_avr_rB0.GetBinContent(i))**2) / math.sqrt(entriesB0[i] - 1)
        rvalueStdB0bar[i] = math.sqrt(histo_ms_rB0bar.GetBinContent(
            i) - (histo_avr_rB0bar.GetBinContent(i))**2) / math.sqrt(entriesB0bar[i] - 1)
        rvalueStdB0Average[i] = math.sqrt(rvalueStdB0[i]**2 + rvalueStdB0bar[i]**2) / 2
        # math.sqrt(histo_ms_r.GetBinContent(i) - (histo_avr_r.GetBinContent(i))**2)
        # calculate the wrong tag fractin (only true if MC data good)

        wvalueB0[i] = histo_mc_NwB0.GetBinContent(i) / entriesB0[i]  # (1 - rvalueB0[i]) / 2
        wvalueB0bar[i] = histo_mc_NwB0bar.GetBinContent(i) / entriesB0bar[i]  # (1 - rvalueB0bar[i]) / 2
        wvalueDiff[i] = wvalueB0[i] - wvalueB0bar[i]
        wvalueB0Uncertainty[i] = math.sqrt(histo_mc_NwB0.GetBinContent(
            i) * (entriesB0[i] - histo_mc_NwB0.GetBinContent(i)) / (entriesB0[i]**3))
        wvalueB0barUncertainty[i] = math.sqrt(histo_mc_NwB0bar.GetBinContent(
            i) * (entriesB0bar[i] - histo_mc_NwB0bar.GetBinContent(i)) / (entriesB0bar[i]**3))
        # math.sqrt((rvalueStdB0[i] / 2)**2 + (rvalueStdB0bar[i] / 2)**2)
        wvalueDiffUncertainty[i] = math.sqrt(wvalueB0Uncertainty[i]**2 + wvalueB0barUncertainty[i]**2)
        wvalue[i] = (wvalueB0[i] + wvalueB0bar[i]) / 2
        wvalueUncertainty[i] = wvalueDiffUncertainty[i] / 2

        # Avr Efficiency

        # iEffEfficiency[i] = (event_fractionB0[i] * rvalueB0[i] * rvalueB0[i] +
        #                                   event_fractionB0bar[i] * rvalueB0bar[i] * rvalueB0bar[i]) / 2

        iEffEfficiency[i] = event_fractionTotal[i] * (1 - 2 * wvalue[i])**2

        iEffEfficiencyUncertainty[i] = (1 - 2 * wvalue[i]) * \
            math.sqrt((2 * event_fractionTotal[i] * 2 * wvalueUncertainty[i])**2 +
                      (1 - 2 * wvalue[i])**2 * event_fractionTotalUncertainty[i]**2)

        # iEffEfficiencyUncertainty[i] = rvalueB0Average[i] * \
        #     math.sqrt((2 * total_entries * entries[i] * rvalueStdB0Average[i])**2 +
        #               rvalueB0Average[i]**2 * entries[i] *
        #               (total_entries * (total_entries - entries[i]) +
        #                entries[i] * total_notTagged)) / (total_entries**2)

        # iEffEfficiencyB0UncertaintyFromOutput[i] = rvalueB0[i] * \
        #     math.sqrt((2 * total_entriesB0 * entriesB0[i] * rvalueStdB0[i])**2 +
        #               rvalueB0[i]**2 * entriesB0[i] *
        #               total_entriesB0 * (total_entriesB0 - entriesB0[i])) / (total_entriesB0**2)
        # iEffEfficiencyB0barUncertaintyFromOutput[i] = rvalueB0bar[i] * \
        #     math.sqrt((2 * total_entriesB0bar * entriesB0bar[i] * rvalueStdB0bar[i])**2 +
        #               rvalueB0bar[i]**2 * entriesB0bar[i] *
        #               total_entriesB0bar * (total_entriesB0bar - entriesB0bar[i])) / (total_entriesB0bar**2)

        # iEffEfficiencyUncertainty[i] =
        # math.sqrt(iEffEfficiencyB0UncertaintyFromOutput[i]**2 +
        # iEffEfficiencyB0barUncertaintyFromOutput[i]**2) / 2

        # iEffEfficiency[i] = (event_fractionB0[i] * (1-2*wvalueB0[i])**2 + event_fractionB0bar[i] * (1-2*wvalueB0bar[i])**2)/2

        average_eff_eff += iEffEfficiency[i]
        # average_eff_eff += (event_fractionB0[i] * (1-2*wvalueB0[i])**2 + event_fractionB0bar[i] * (1-2*wvalueB0bar[i])**2)/2

        # Delta Eff

        iDeltaEffEfficiency[i] = event_fractionB0[i] * (1 - 2 * wvalueB0[i])**2 - \
            event_fractionB0bar[i] * (1 - 2 * wvalueB0bar[i])**2

        iEffEfficiencyB0Uncertainty[i] = (1 - 2 * wvalueB0[i]) * \
            math.sqrt((2 * total_entriesB0 * entriesB0[i] * 2 * wvalueB0Uncertainty[i])**2 +
                      (1 - 2 * wvalueB0[i])**2 * entriesB0[i] *
                      total_entriesB0 * (total_entriesB0 - entriesB0[i])) / (total_entriesB0**2)
        iEffEfficiencyB0barUncertainty[i] = (1 - 2 * wvalueB0bar[i]) * \
            math.sqrt((2 * total_entriesB0bar * entriesB0bar[i] * 2 * wvalueB0barUncertainty[i])**2 +
                      (1 - 2 * wvalueB0bar[i])**2 * entriesB0bar[i] *
                      total_entriesB0bar * (total_entriesB0bar - entriesB0bar[i])) / (total_entriesB0bar**2)

        iDeltaEffEfficiencyUncertainty[i] = math.sqrt(iEffEfficiencyB0Uncertainty[i]**2 + iEffEfficiencyB0barUncertainty[i]**2)
        # iEffEfficiencyUncertainty[i] = iDeltaEffEfficiencyUncertainty[i]/2

        diff_eff_Uncertainty = diff_eff_Uncertainty + iDeltaEffEfficiencyUncertainty[i]**2

        # finally calculating the total effective efficiency
        tot_eff_effB0 = tot_eff_effB0 + event_fractionB0[i] * (1 - 2 * wvalueB0[i])**2
        tot_eff_effB0bar = tot_eff_effB0bar + event_fractionB0bar[i] * (1 - 2 * wvalueB0bar[i])**2
        uncertainty_eff_effAverage = uncertainty_eff_effAverage + iEffEfficiencyUncertainty[i]**2
        uncertainty_eff_effB0 = uncertainty_eff_effB0 + iEffEfficiencyB0Uncertainty[i]**2
        uncertainty_eff_effB0bar = uncertainty_eff_effB0bar + iEffEfficiencyB0barUncertainty[i]**2
        muParam[i] = event_fractionDiff[i] / (2 * event_fractionTotal[i])
        muParamUncertainty[i] = event_fractionDiffUncertainty[i] / (2 * event_fractionTotal[i]) * math.sqrt(muParam[i]**2 + 1)

        # intervallEff[i] = event_fractionTotal[i] * rvalueB0Average[i] * rvalueB0Average[i]
        print('* ' + '{:.3f}'.format(r_subsample[i - 1]) + ' - ' + '{:.3f}'.format(r_subsample[i]) + '   ' +
              '{:.3f}'.format(rvalueB0Average[i]) + ' +- ' + '{:.4f}'.format(rvalueStdB0Average[i]) + '    ' +
              '{:.4f}'.format(event_fractionTotal[i]) + ' ' +
              '{: .4f}'.format(event_fractionDiff[i]) + ' +- ' + '{:.4f}'.format(event_fractionDiffUncertainty[i]) + ' ' +
              '{: .4f}'.format(muParam[i]) + ' +- ' + '{:.4f}'.format(muParamUncertainty[i]) + '   ' +
              '{:.4f}'.format(wvalue[i]) + ' +- ' + '{:.4f}'.format(wvalueUncertainty[i]) + '  ' +
              '{: .4f}'.format(wvalueDiff[i]) + ' +- ' + '{:.4f}'.format(wvalueDiffUncertainty[i]) + '  *')

    # average_eff_eff = (tot_eff_effB0 + tot_eff_effB0bar) / 2
    uncertainty_eff_effAverage = math.sqrt(uncertainty_eff_effAverage)
    uncertainty_eff_effB0 = math.sqrt(uncertainty_eff_effB0)
    uncertainty_eff_effB0bar = math.sqrt(uncertainty_eff_effB0bar)
    diff_eff = tot_eff_effB0 - tot_eff_effB0bar
    diff_eff_Uncertainty = math.sqrt(diff_eff_Uncertainty)
    print('* --------------------------------------------------------------------------------------------------' +
          '------------------ *')
    print('*                                                                                                  *')
    print('*    __________________________________________________________________________________________    *')
    print('*   |                                                                                          |   *')
    print('*   | TOTAL NUMBER OF TAGGED EVENTS  =  ' +
          '{:<24}'.format("%.0f" % total_tagged) + '{:>36}'.format('|   *'))
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFICIENCY  (q=+-1)=  ' +
        '{:.2f}'.format(
            tagging_eff *
            100) +
        " +- " +
        '{:.2f}'.format(
            DeltaTagging_eff *
            100) +
        ' %                                      |   *')
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY  (q=+-1)=  ' +
        '{:.6f}'.format(
            average_eff_eff *
            100) +
        " +- " +
        '{:.6f}'.format(
            uncertainty_eff_effAverage *
            100) +
        ' %                    |   *')
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY ASYMMETRY (q=+-1)=  ' +
        '{:^9.6f}'.format(
            diff_eff *
            100) +
        " +- " +
        '{:.6f}'.format(
            diff_eff_Uncertainty *
            100) +
        ' %           |   *')
    print('*   |                                                                                          |   *')
    print('*   | B0-TAGGER  TOTAL EFFECTIVE EFFICIENCIES: ' +
          '{:.2f}'.format(tot_eff_effB0 * 100) + " +-" + '{: 4.2f}'.format(uncertainty_eff_effB0 * 100) +
          ' % (q=+1)  ' +
          '{:.2f}'.format(tot_eff_effB0bar * 100) + " +-" + '{: 4.2f}'.format(uncertainty_eff_effB0bar * 100) +
          ' % (q=-1) ' + ' |   *')
    print('*   |                                                                                          |   *')
    print('*   | FLAVOR PERCENTAGE (MC):                  ' +
          '{:.2f}'.format(total_tagged_B0 / total_tagged * 100) + ' % (q=+1)  ' +
          '{:.2f}'.format(total_tagged_B0bar / total_tagged * 100) + ' % (q=-1)  Diff=' +
          '{:^5.2f}'.format((total_tagged_B0 - total_tagged_B0bar) / total_tagged * 100) + ' %    |   *')
    print('*   |__________________________________________________________________________________________|   *')
    print('*                                                                                                  *')
    print('****************************************************************************************************')
    print('*                                                                                                  *')

    # not that imortant
    print('* ---------------------------------                                                                *')
    print('* Efficiency Determination - easiest way                                                           *')
    print('* ---------------------------------                                                                *')
    total_tagged_B0 = histo_belleplotB0.GetEntries()
    total_tagged_B0Bar = histo_belleplotB0bar.GetEntries()
    total_tagged_wrong = histo_m1.GetEntries()
    total_tagged_B0Bar_wrong = histo_m2.GetEntries()
    total_tagged = total_tagged_B0 + total_tagged_B0Bar
    total_tagged_wrong = total_tagged_wrong + total_tagged_B0Bar_wrong

    wrong_tag_fraction_B0 = total_tagged_wrong / total_tagged_B0
    wrong_tag_fraction_B0Bar = total_tagged_B0Bar_wrong / total_tagged_B0Bar
    wrong_tag_fraction = total_tagged_wrong / total_tagged
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
    print('****************************************************************************************************')
    print('')
    print('Table For B2TIP')
    print('')
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
    Canvas1 = ROOT.TCanvas('Bla' + method, 'Final Output', 1200, 800)
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
    Canvas2 = ROOT.TCanvas('Bla2' + method, 'Calibration plot for true B0', 1200, 800)
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
    histo_ms_r.Delete()
    histo_ms_rB0.Delete()
    histo_ms_rB0bar.Delete()
    histo_mc_NwB0.Delete()
    histo_mc_NwB0bar.Delete()
    histo_notTaggedEvents.Delete()
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

    print(r'\begin{tabularx}{1\textwidth}{@{}r  r  r  r  r  r  r@{}}')
    print(r'\hline')
    print(r'$r$- Interval $\enskip$ & $\varepsilon_i\ $ &  $\Delta\varepsilon_i\ $ & $w_i \pm \delta w_i\enskip\; $ ' +
          r' & $\Delta w_i \pm \delta\Delta w_i $& $\varepsilon_{\text{eff}, i} \pm \delta\varepsilon_{\text{eff}, i}\enskip\, $ ' +
          r' & $\Delta \varepsilon_{\text{eff}, i}  \pm \delta\Delta \varepsilon_{\text{eff}, i}\enskip\, $\\ \hline\hline')
    for i in range(1, r_size):
        print('$ ' + '{:.3f}'.format(r_subsample[i - 1]) + ' - ' + '{:.3f}'.format(r_subsample[i]) + '$ & $'
              '{: 6.1f}'.format(event_fractionTotal[i] * 100) + r'$ & $' +
              '{: 7.2f}'.format(event_fractionDiff[i] * 100) + r'\;$ & $' +
              '{: 7.2f}'.format(wvalue[i] * 100) + r" \pm " + '{:2.2f}'.format(wvalueUncertainty[i] * 100) + r'\enskip $ & $' +
              '{: 7.2f}'.format(wvalueDiff[i] * 100) + r" \pm " +
              '{:2.2f}'.format(wvalueDiffUncertainty[i] * 100) + r'\enskip $ & $' +
              '{: 8.4f}'.format(iEffEfficiency[i] * 100) +  # + '$ & $' +
              r" \pm " + '{:2.4f}'.format(iEffEfficiencyUncertainty[i] * 100) + r'\, $ & $' +
              '{: 6.4f}'.format(iDeltaEffEfficiency[i] * 100) +  # +
              r" \pm " + '{:2.4f}'.format(iDeltaEffEfficiencyUncertainty[i] * 100) +
              r'\enskip\enskip $ \\ ')
    print(r'\hline\hline')
    print(r'\multicolumn{1}{r}{Total} &  & \multicolumn{5}{r}{ $\varepsilon_\text{eff} = ' +
          r'\sum_i \varepsilon_i \cdot \langle 1-2w_i\rangle^2 = ' +
          '{: 6.2f}'.format(average_eff_eff * 100) + r" \pm " + '{: 6.2f}'.format(uncertainty_eff_effAverage * 100) + r'\enskip\, ')
    print(r'\Delta \varepsilon_\text{eff} = ' +
          '{: 6.2f}'.format(diff_eff * 100) + r" \pm " + '{: 6.2f}'.format(diff_eff_Uncertainty * 100) + r'\quad\  $ }' +
          r' \\ ')
    print(r'\hline')
    print(r'\end{tabular}')
    print('')

    print('')
    print('Mu-Values for Table')
    print('')

    for i in range(1, r_size):
        print('$ ' + '{:.3f}'.format(r_subsample[i - 1]) + ' - ' + '{:.3f}'.format(r_subsample[i]) + '$ & $'
              '{: 7.2f}'.format(muParam[i] * 100) + r" \pm " + '{:2.2f}'.format(muParamUncertainty[i] * 100) + r' $ & ')
# **********************************************
# DETERMINATION OF INDIVIDUAL EFFECTIVE EFFICIENCY
# **********************************************

# keep in mind:
# the individual efficiency is determind on basis of the combiner training.
# Whereas the efficiency is determined on basis of the final expert output.
# print(eventLevelParticleLists)
# eventLevelParticleLists.append(('K+:inRoe', 'KaonNotWeighted',
#                                 'QpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))'))
# eventLevelParticleLists.append(('Lambda0:inRoe', 'LambdaNotWeighted',
#                                 'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))'))
# eventLevelParticleLists.append(('e+:inRoe', 'Electron',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))'))
# eventLevelParticleLists.append(
#     ('e+:inRoe',
#      'IntermediateElectron',
#      'weightedQpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))'))
# eventLevelParticleLists.append(('mu+:inRoe', 'Muon',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(Muon), isRightCategory(Muon))'))
# eventLevelParticleLists.append(('mu+:inRoe', 'IntermediateMuon',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))'))
# eventLevelParticleLists.append(('mu+:inRoe', 'KinLepton',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))'))
# eventLevelParticleLists.append(
#     ('mu+:inRoe',
#      'IntermediateKinLepton',
#      'weightedQpOf(e+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))'))
# eventLevelParticleLists.append(('pi+:inRoe', 'SlowPion',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))'))
# eventLevelParticleLists.append(('pi+:inRoe', 'FSC',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))'))
# eventLevelParticleLists.append(('pi+:inRoe', 'MaximumPstar',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))'))
# eventLevelParticleLists.append(('pi+:inRoe', 'FastHadron',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))'))
# eventLevelParticleLists.append(('K+:inRoe', 'KaonPion',
#                                 'weightedQpOf(e+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))'))


print(ft.eventLevelParticleLists)


print('******************************************* MEASURED EFFECTIVE EFFICIENCY FOR INDIVIDUAL CATEGORIES ' +
      '**********************************************')
print('*                                                                                                   ' +
      '                                             *')
# input: Classifier input from event-level. Output of event-level is recalculated for input on combiner-level.
# but is re-evaluated under combiner target. Signal is B0, background is B0Bar.
categoriesPerformance = []
NbinsCategories = 100
for (particleList, category, combinerVariable) in ft.eventLevelParticleLists:
    # histogram of input variable (only signal) - not yet a probability! It's a classifier plot!
    hist_both = ROOT.TH1F('Both_' + category, 'Input Both (B0) ' +
                          category + ' (binning)', NbinsCategories, -1.0, 1.0)
    # histogram of input variable (only signal) - not yet a probability! It's a classifier plot!
    hist_signal = ROOT.TH1F('Signal_' + category, 'Input Signal (B0) ' +
                            category + ' (binning)', NbinsCategories, -1.0, 1.0)
    # histogram of input variable (only background) - not yet a probability! It's a classifier plot!
    hist_background = ROOT.TH1F('Background_' + category, 'Input Background (B0bar) ' +
                                category + ' (binning)', NbinsCategories, -1.0, 1.0)

    # per definiton that input is not comparable to the network output, this has to be transformed.
    # probability output from 0 to 1 (corresponds to net output probability) -> calculation below
    hist_probB0 = ROOT.TH1F('Probability' + category,
                            'Transformed to probability (B0) (' + category + ')',
                            NbinsCategories, 0.0, 1.0)
    hist_probB0bar = ROOT.TH1F('ProbabilityB0bar_' + category,
                               'Transformed to probability (B0bar) (' + category + ')',
                               NbinsCategories, 0.0, 1.0)
    # qp output from -1 to 1 -> transformation below
    hist_qrB0 = ROOT.TH1F('QR' + category, 'Transformed to qp (B0)(' +
                          category + ')', NbinsCategories, -1.0, 1.0)
    hist_qrB0bar = ROOT.TH1F('QRB0bar_' + category, 'Transformed to qp (B0bar) (' +
                             category + ')', NbinsCategories, -1.0, 1.0)
    # histogram for abs(qp), i.e. this histogram contains the r-values -> transformation below
    # also used to get the number of entries, sorted into 6 bins
    histo_entries_per_bin = ROOT.TH1F('entries_per_bin_' + category, 'Abs(qp)(B0) (' + category + ')', int(r_size - 2), r_subsample)
    histo_entries_per_binB0 = ROOT.TH1F('entries_per_bin' + category, 'Abs(qp)(B0) (' +
                                        category + ')', int(r_size - 2), r_subsample)
    histo_entries_per_binB0bar = ROOT.TH1F('entries_per_binB0bar_' + category,
                                           'Abs(qp) (B0bar) (' + category + ')', int(r_size - 2), r_subsample)

    # histogram contains at the end the average r values -> calculation below
    # sorted into r bins
    hist_avr_rB0 = ROOT.TH1F('Average_r' + category, 'Average r for B0' +
                             category, int(r_size - 2), r_subsample)
    hist_avr_rB0bar = ROOT.TH1F('Average_rB0bar_' + category, 'Average r for B0bar' +
                                category, int(r_size - 2), r_subsample)

    hist_ms_rB0 = ROOT.TH1F('AverageSqrdR' + category, 'Average r sqrd for B0' +
                            category, int(r_size - 2), r_subsample)
    hist_ms_rB0bar = ROOT.TH1F('AverageSqrdRB0bar_' + category, 'Average r sqrd for B0bar' +
                               category, int(r_size - 2), r_subsample)

    # ****** TEST OF CALIBRATION ******
    # for calibration plot we want to have
    hist_all = ROOT.TH1F('All_' + category, 'Input Signal (B0) and Background (B0Bar)' +
                         category + ' (binning 50)', 50, 0.0, 1.0)
    hist_calib_B0 = ROOT.TH1F('Calib_' + category, 'Calibration Plot for true B0' +
                              category + ' (binning 50)', 50, 0.0, 1.0)

    # fill both
    if category != "KaonNotWeighted" and category != "LambdaNotWeighted":
        #  if category == 'Lambda' or combinerVariable.find('weighted') == -1:
        tree.Draw('qp' + category + '>>Both_' + category, 'abs(qrMC) == 1.0')
        # fill signal
        tree.Draw('qp' + category + '>>Signal_' + category, 'qrMC == 1.0')
        # fill background
        tree.Draw('qp' + category + '>>Background_' + category, 'qrMC == -1.0')
        tree.Draw('qp' + category + '>>All_' + category, 'qrMC!=0')
        tree.Draw('qp' + category + '>>Calib_' + category, 'qrMC == 1.0')

        # elif combinerVariable.find('weighted') != -1:
        #     tree.Draw('extraInfo__boWeightedQpOf' + category + '__bc' + '>>Both_' + category, 'abs(qrMC) == 1.0')
        #     tree.Draw('extraInfo__boWeightedQpOf' + category + '__bc' + '>>Signal_' + category, 'qrMC == 1.0')
        #     tree.Draw('extraInfo__boWeightedQpOf' + category + '__bc' + '>>Background_' + category, 'qrMC == -1.0')
        #     tree.Draw('extraInfo__boWeightedQpOf' + category + '__bc' + '>>All_' + category, 'qrMC!=0')
        #     tree.Draw('extraInfo__boWeightedQpOf' + category + '__bc' + '>>Calib_' + category, 'qrMC == 1.0')
        #     category = category + 'W'

    elif category == "KaonNotWeighted":
        tree.Draw('extraInfo__boQpOfKaon__bc' + '>>Both_' + category, 'abs(qrMC) == 1.0')
        tree.Draw('extraInfo__boQpOfKaon__bc' + '>>Signal_' + category, 'qrMC == 1.0')
        tree.Draw('extraInfo__boQpOfKaon__bc' + '>>Background_' + category, 'qrMC == -1.0')
        tree.Draw('extraInfo__boQpOfKaon__bc' + '>>All_' + category, 'qrMC!=0')
        tree.Draw('extraInfo__boQpOfKaon__bc' + '>>Calib_' + category, 'qrMC == 1.0')

    elif category == "LambdaNotWeighted":
        tree.Draw('extraInfo__boQpOfLambda__bc' + '>>Both_' + category, 'abs(qrMC) == 1.0')
        tree.Draw('extraInfo__boQpOfLambda__bc' + '>>Signal_' + category, 'qrMC == 1.0')
        tree.Draw('extraInfo__boQpOfLambda__bc' + '>>Background_' + category, 'qrMC == -1.0')
        tree.Draw('extraInfo__boQpOfLambda__bc' + '>>All_' + category, 'qrMC!=0')
        tree.Draw('extraInfo__boQpOfLambda__bc' + '>>Calib_' + category, 'qrMC == 1.0')

    hist_calib_B0.Divide(hist_all)
    # ****** produce the input plots from combiner level ******

    maxSignal = hist_signal.GetBinContent(hist_signal.GetMaximumBin())
    maxBackground = hist_background.GetBinContent(hist_background.GetMaximumBin())

    Ymax = max(maxSignal, maxBackground)
    Ymax = Ymax + Ymax / 12

    ROOT.gStyle.SetOptStat(0)
    with Quiet(ROOT.kError):
        Canvas = ROOT.TCanvas('Bla', 'TITEL BLA', 1200, 800)
    Canvas.cd()  # activate
    Canvas.SetLogy()
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

    hist_signal.SetTitle(category + ' category; #it{qp}-Output ; Events')
    # hist_signal.SetMinimum(0)
    hist_signal.SetMaximum(Ymax)
    # hist_background.SetMinimum(0)
    hist_background.SetMaximum(Ymax)

    hist_signal.Draw('hist')
    hist_background.Draw('hist same')

    if category == 'MaximumPstar':
        legend = ROOT.TLegend(0.4, 0.75, 0.6, 0.9)
    else:
        legend = ROOT.TLegend(0.6, 0.75, 0.8, 0.9)
    legend.AddEntry(hist_signal, 'true B0')
    legend.AddEntry(hist_background, 'true B0bar')
    legend.SetTextSize(0.05)
    legend.Draw()

    Canvas.Update()
    with Quiet(ROOT.kError):
        Canvas.SaveAs(workingDirectory + '/' + 'PIC_' + category + '_Input_Combiner.pdf')

    # ***** TEST OF CALIBRATION ******

    # initialize some arrays
    binCounter = int(NbinsCategories + 1)
    dilutionB02 = array('d', [0] * binCounter)
    dilutionB0bar2 = array('d', [0] * binCounter)
    purityB0 = array('d', [0] * binCounter)
    purityB0bar = array('d', [0] * binCounter)
    signal = array('d', [0] * binCounter)
    back = array('d', [0] * binCounter)
    weight = array('d', [0] * binCounter)

    for i in range(1, binCounter):
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

        # filling histogram with abs(qr), i.e. this histogram contains the r-values (not qp)
        histo_entries_per_binB0.Fill(abs(dilutionB02[i]), signal[i])
        histo_entries_per_binB0bar.Fill(abs(dilutionB0bar2[i]), back[i])
        # filling histogram with abs(qr) special weighted - needed for average r calculation
        hist_avr_rB0.Fill(abs(dilutionB02[i]), abs(dilutionB02[i]) * signal[i])
        hist_avr_rB0bar.Fill(abs(dilutionB0bar2[i]), abs(dilutionB0bar2[i]) * back[i])
        # filling histogram with abs(qr)**2 special weighted - needed for std dev of r calculation
        hist_ms_rB0.Fill(abs(dilutionB02[i]), abs(dilutionB02[i] * dilutionB02[i]) * signal[i])
        hist_ms_rB0bar.Fill(abs(dilutionB0bar2[i]), abs(dilutionB0bar2[i] * dilutionB02[i]) * back[i])

    # hist_avr_rB0bar contains now the average r-value
    hist_avr_rB0.Divide(histo_entries_per_binB0)
    hist_avr_rB0bar.Divide(histo_entries_per_binB0bar)

    hist_ms_rB0.Divide(histo_entries_per_binB0)
    hist_ms_rB0bar.Divide(histo_entries_per_binB0bar)
    # now calculating the efficiency

    # calculating number of events
    total_entriesB0 = total_notTagged / 2
    total_entriesB0bar = total_notTagged / 2
    for i in range(1, r_size):
        total_entriesB0 = total_entriesB0 + histo_entries_per_binB0.GetBinContent(i)
        total_entriesB0bar = total_entriesB0bar + histo_entries_per_binB0bar.GetBinContent(i)
    # initializing some arrays
    tot_eff_effB0 = 0
    tot_eff_effB0bar = 0
    uncertainty_eff_effB0 = 0
    uncertainty_eff_effB0bar = 0
    uncertainty_eff_effAverage = 0
    diff_eff_Uncertainty = 0
    event_fractionB0 = array('f', [0] * r_size)
    event_fractionB0bar = array('f', [0] * r_size)
    rvalueB0 = array('f', [0] * r_size)
    rvalueB0bar = array('f', [0] * r_size)
    rvalueStdB0 = array('f', [0] * r_size)
    rvalueStdB0bar = array('f', [0] * r_size)
    # wvalue = array('f', [0] * r_size)
    entriesBoth = array('f', [0] * r_size)
    entriesB0 = array('f', [0] * r_size)
    entriesB0bar = array('f', [0] * r_size)
    iEffEfficiencyB0Uncertainty = array('f', [0] * r_size)
    iEffEfficiencyB0barUncertainty = array('f', [0] * r_size)
    iDeltaEffEfficiencyUncertainty = array('f', [0] * r_size)

    for i in range(1, r_size):

        entriesBoth[i] = entriesB0bar[i] + entriesB0[i]
        entriesB0[i] = histo_entries_per_binB0.GetBinContent(i)
        entriesB0bar[i] = histo_entries_per_binB0bar.GetBinContent(i)
        event_fractionB0[i] = entriesB0[i] / total_entriesB0
        event_fractionB0bar[i] = entriesB0bar[i] / total_entriesB0bar
        # print '*  Bin ' + str(i) + ' r-value: ' + str(rvalueB0[i]), 'entriesB0: ' +
        # str(event_fractionB0[i] * 100) + ' % (' + str(entriesB0[i]) + '/' +
        # str(total_entriesB0) + ')'

        rvalueB0[i] = hist_avr_rB0.GetBinContent(i)
        rvalueB0bar[i] = hist_avr_rB0bar.GetBinContent(i)

        rvalueStdB0[i] = 0
        rvalueStdB0bar[i] = 0

        if entriesB0[i] > 1:
            rvalueStdB0[i] = math.sqrt(abs(hist_ms_rB0.GetBinContent(
                i) - (hist_avr_rB0.GetBinContent(i))**2)) / math.sqrt(entriesB0[i] - 1)

        if entriesB0bar[i] > 1:
            rvalueStdB0bar[i] = math.sqrt(abs(hist_ms_rB0bar.GetBinContent(
                i) - (hist_avr_rB0bar.GetBinContent(i))**2)) / math.sqrt(entriesB0bar[i] - 1)
        # wvalue[i] = (1 - rvalueB0[i]) / 2

        tot_eff_effB0 = tot_eff_effB0 + event_fractionB0[i] * rvalueB0[i] \
            * rvalueB0[i]
        tot_eff_effB0bar = tot_eff_effB0bar + event_fractionB0bar[i] * rvalueB0bar[i] \
            * rvalueB0bar[i]

        iEffEfficiencyB0Uncertainty[i] = rvalueB0[i] * \
            math.sqrt((2 * total_entriesB0 * entriesB0[i] * rvalueStdB0[i])**2 +
                      rvalueB0[i]**2 * entriesB0[i] *
                      (total_entriesB0 * (total_entriesB0 - entriesB0[i]) +
                       entriesB0[i] * total_notTagged)) / (total_entriesB0**2)
        iEffEfficiencyB0barUncertainty[i] = rvalueB0bar[i] * \
            math.sqrt((2 * total_entriesB0bar * entriesB0bar[i] * rvalueStdB0bar[i])**2 +
                      rvalueB0bar[i]**2 * entriesB0bar[i] *
                      (total_entriesB0bar * (total_entriesB0bar - entriesB0bar[i]) +
                       entriesB0bar[i] * total_notTagged)) / (total_entriesB0bar**2)

        iDeltaEffEfficiencyUncertainty[i] = math.sqrt(iEffEfficiencyB0Uncertainty[i]**2 + iEffEfficiencyB0barUncertainty[i]**2)

        diff_eff_Uncertainty = diff_eff_Uncertainty + iDeltaEffEfficiencyUncertainty[i]**2

        uncertainty_eff_effB0 = uncertainty_eff_effB0 + iEffEfficiencyB0Uncertainty[i]**2
        uncertainty_eff_effB0bar = uncertainty_eff_effB0bar + iEffEfficiencyB0barUncertainty[i]**2

    effDiff = tot_eff_effB0 - tot_eff_effB0bar
    effAverage = (tot_eff_effB0 + tot_eff_effB0bar) / 2

    uncertainty_eff_effB0 = math.sqrt(uncertainty_eff_effB0)
    uncertainty_eff_effB0bar = math.sqrt(uncertainty_eff_effB0bar)
    diff_eff_Uncertainty = math.sqrt(diff_eff_Uncertainty)
    uncertainty_eff_effAverage = diff_eff_Uncertainty / 2
    print(
        '{:<25}'.format("* " + category) + ' B0-Eff=' +
        '{: 8.2f}'.format(tot_eff_effB0 * 100) + " +-" + '{: 4.2f}'.format(uncertainty_eff_effB0 * 100) +
        ' %' +
        '   B0bar-Eff=' +
        '{: 8.2f}'.format(tot_eff_effB0bar * 100) + " +-" + '{: 4.2f}'.format(uncertainty_eff_effB0bar * 100) +
        ' %' +
        '   EffAverage=' +
        '{: 8.2f}'.format(effAverage * 100) + " +- " + '{:4.2f}'.format(uncertainty_eff_effAverage * 100) + ' %' +
        '   EffDiff=' +
        '{: 8.2f}'.format(effDiff * 100) + " +- " + '{:4.2f}'.format(diff_eff_Uncertainty * 100) + ' %  *')

    # hist_signal.Write('', ROOT.TObject.kOverwrite)
    # hist_background.Write('', ROOT.TObject.kOverwrite)
    # hist_probB0.Write('', ROOT.TObject.kOverwrite)
    # hist_probB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_qpB0.Write('', ROOT.TObject.kOverwrite)
    # hist_qpB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_absqpB0.Write('', ROOT.TObject.kOverwrite)
    # hist_absqpB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_avr_rB0.Write('', ROOT.TObject.kOverwrite)
    # hist_avr_rB0bar.Write('', ROOT.TObject.kOverwrite)
    # hist_all.Write('', ROOT.TObject.kOverwrite)
    # hist_calib_B0.Write('', ROOT.TObject.kOverwrite)
    categoriesPerformance.append((category, effAverage, uncertainty_eff_effAverage, effDiff, diff_eff_Uncertainty))
    with Quiet(ROOT.kError):
        Canvas.Clear()
# if average_eff != 0:
    # print '*    -------------------------------------------------------------------------'
    # print '*    ' + '{: > 8.2f}'.format(average_eff * 100) + ' %' \
    # + '{:>85}'.format('TOTAL' + '                      *')

print('*                                                                                                                         ' +
      '                       *')
print('**************************************************************************************************************************' +
      '************************')
print('')
print('Table For B2TIP')
print('')
print(r'\begin{tabular}{ l  r  r }')
print(r'\hline')
print(r'Categories & $\varepsilon_\text{eff} \pm \delta\varepsilon_\text{eff} $& ' +
      r'$\Delta\varepsilon_\text{eff} \pm \delta\Delta\varepsilon_\text{eff}$\\ \hline\hline')
for (category, effAverage, uncertainty_eff_effAverage, effDiff, diff_eff_Uncertainty) in categoriesPerformance:
    print(
        '{:<23}'.format(category) +
        ' & $' +
        '{: 6.2f}'.format(effAverage * 100) + r" \pm " + '{:4.2f}'.format(uncertainty_eff_effAverage * 100) +
        ' $ & $' +
        '{: 6.2f}'.format(effDiff * 100) + r" \pm " + '{:4.2f}'.format(diff_eff_Uncertainty * 100) +
        r'\ \enskip  $ \\')
print(r'\hline')
print(r'\end{tabular}')
B2INFO('qp Output Histograms in pdf format saved at: ' + workingDirectory)
with open("categoriesPerformance.pkl", "wb") as f:
    pickle.dump(categoriesPerformance, f)
