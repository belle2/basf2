#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Author: Moritz Gelb                      *
# * Script for autmating reporting           *
# * flavor tagging                           *
#                                            *
# ********************************************

from basf2 import *
import ROOT
import pdg

import math
import actorFramework
import preCutDetermination
import os
import re
import sys
import subprocess
import copy
from string import Template
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False
import ROOT
from array import array
import IPython

ROOT.gROOT.SetBatch(True)

# TODO: WRITE OUT EVERYTHING IN PDF PLOTS

# working directory - Input file from flavor tagger
# please note: two different input files are needed
# specify file name, if necessary

workingDirectory = Belle2.FileSystem.findFile('/analysis/data/FlavorTagging/RUN13')
B2INFO("Working directory is: " + workingDirectory)

#
# *****************************************
# DETERMINATION OF TOTAL EFFECTIVE EFFIENCY
# *****************************************
#

r_subsample = array('d', [0.0, 0.25, 0.5, 0.625, 0.75, 0.875, 1.0])
overall_eff = 0

# working directory
# needs the B0_B0bar_final.root-file
if Belle2.FileSystem.findFile(workingDirectory + '/B0_B0bar_final.root'):
    # root-file
    rootfile = ROOT.TFile(workingDirectory + '/B0_B0bar_final.root', 'UPDATE')
    tree = rootfile.Get('TaggingInformation')

    mcstatus = array('d', [-511.5, 0.0, 511.5])
    rootfile.cd()
    ROOT.TH1.SetDefaultSumw2()
    #bekommt man mit GetBinError(), setzten mit SetBinError()
    # histogram contains the average r in each of 6 bins -> calculation see below
    histo_avr_r = ROOT.TH1F('Average_r', 'Average r in each of 6 bins', 6, r_subsample)
    # histogram with number of entries in for each bin
    histo_entries_per_bin = ROOT.TH1F('entries_per_bin', 'Events binned in r_subsample according to their r-value for B0 prob', 6, r_subsample)
    # histogram network output (not qr and not r) for true B0 (signal) - not necessary
    histo_Cnet_output_B0 = ROOT.TH1F('Comb_Net_Output_B0', 'Combiner network output [not equal to r] for true B0 (binning 100)', 100, 0.0, 1.0)
    # histogram network output (not qr and not r) for true B0bar (background) - not necessary
    histo_Cnet_output_B0bar = ROOT.TH1F('Comb_Net_Output_B0bar', 'Combiner network output [not equal to r] for true B0bar (binning 100)', 100, 0.0, 1.0)
    # histogram containing the belle paper plot (qr-tagger output for true B0)
    histo_belleplotB0 = ROOT.TH1F('BellePlot_B0', 'BellePlot for true B0 (binning 50)', 50, -1.0, 1.0)
    # histogram containing the belle paper plot (qr-tagger output for true B0bar)
    histo_belleplotB0bar = ROOT.TH1F('BellePlot_B0Bar', 'BellePlot for true B0Bar (binning 50)', 50, -1.0, 1.0)
    # calibration plot for B0. If we get a linaer line our MC is fine, than the assumption r ~ 1- 2w is reasonable
    # expectation is, that for B0 calibration plot:  qr=0  half B0 and half B0bar, qr = 1 only B0 and qr = -1
    # no B0. Inverse for B0bar calibration plot
    histo_calib_B0 = ROOT.TH1F('Calibration_B0', 'CalibrationPlot for true B0 (binning 100)', 100, -1.0, 1.0)
    # calibration plot for B0bar calibration plot
    histo_calib_B0bar = ROOT.TH1F('Calibration_B0Bar', 'CalibrationPlot for true B0Bar (binning 100)', 100, -1.0, 1.0)
    # belle plot with true B0 and B0bars
    hallo12 = ROOT.TH1F('BellePlot_NoCut', 'BellePlot_NoCut (binning 100)', 100, -1.0, 1.0)

    ###############################
    diag = ROOT.TF1('diag', 'pol1')
    ###############################

    # histograms for the efficiency calculation in wrong way
    histo_m0 = ROOT.TH1F('BellePlot_B0_m0', 'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m1 = ROOT.TH1F('BellePlot_B0_m1', 'BellePlot_m for true B0 (binning 50)', 50, -1.0, 1.0)
    histo_m2 = ROOT.TH1F('BellePlot_B0_m2', 'BellePlot_m for true B0Bar (binning 50)', 50, -1.0, 1.0)

    # filling the histograms

    # filling with abs(qr) in one of 6 bins with its weight
    tree.Project('Average_r', 'abs(getExtraInfoqr_Combined)', 'abs(getExtraInfoqr_Combined)')
    # filling with abs(qr) in one of 6 bins
    tree.Project('entries_per_bin', 'abs(getExtraInfoqr_Combined)')

    # not necessary
    tree.Draw('getExtraInfoB0_prob>>Comb_Net_Output_B0', 'McFlavorOfTagSide>0')
    tree.Draw('getExtraInfoB0_prob>>Comb_Net_Output_B0bar', 'McFlavorOfTagSide<0')

    tree.Draw('getExtraInfoqr_Combined>>BellePlot_B0', 'McFlavorOfTagSide>0 ')
    tree.Draw('getExtraInfoqr_Combined>>BellePlot_B0Bar', 'McFlavorOfTagSide<0')
    tree.Draw('getExtraInfoqr_Combined>>BellePlot_NoCut')

    tree.Draw('getExtraInfoqr_Combined>>Calibration_B0', 'McFlavorOfTagSide>0')
    tree.Draw('getExtraInfoqr_Combined>>Calibration_B0Bar', 'McFlavorOfTagSide<0')

    # filling histograms wrong efficiency calculation
    tree.Draw('getExtraInfoqr_Combined>>BellePlot_B0_m0', 'McFlavorOfTagSide>0 && getExtraInfoqr_Combined>0')
    tree.Draw('getExtraInfoqr_Combined>>BellePlot_B0_m1', 'McFlavorOfTagSide>0 && getExtraInfoqr_Combined<0')
    tree.Draw('getExtraInfoqr_Combined>>BellePlot_B0_m2', 'McFlavorOfTagSide<0 && getExtraInfoqr_Combined>0')

    # producing the average r histogram
    histo_avr_r.Divide(histo_entries_per_bin)

    # producing the calibration plots
    # Errors ok
    histo_calib_B0.Divide(hallo12)
    histo_calib_B0bar.Divide(hallo12)

    #Fit for calibration plot
    print ' '
    print '****************** CALIBRATION CHECK ****************************************************************'
    print ' '
    print 'Fit ploynomial of first order to the calibration plot. Expected value ~0.5'
    print ' '
    histo_calib_B0.Fit(diag, 'TEST')
    print '       '
    print '****************** MEASURED EFFECTIVE EFFICIENCY ***************************************************'
    print '*                                                                                                  *'
    # get total number of entries
    total_entries_B0 = histo_entries_per_bin.GetEntries()
    tot_eff_eff_B0 = 0
    event_fraction_B0 = array('f', [0] * 7)
    rvalueB0 = array('f', [0] * 7)
    #wvalueB0 = array('f', [0]*7)
    entriesB0 = array('f', [0] * 7)
    print '*                 -->  DETERMINATION BASED ON MONTE CARLO                                          *'
    print '*                                                                                                  *'
    for i in range(1, 7):
        # get the average r-value
        rvalueB0[i] = histo_avr_r.GetBinContent(i)
        # calculate the wrong tag fractin (only true if MC data good)
        #wvalueB0[i] = (1 - rvalueB0[i])/2
        entriesB0[i] = histo_entries_per_bin.GetBinContent(i)
        # fraction of events/all events
        event_fraction_B0[i] = entriesB0[i] / total_entries_B0
        print '*  Bin ' + str(i) + '    r-value: ' + '{:.3f}'.format(rvalueB0[i]), '    entries: ' + '{:.3f}'.format(event_fraction_B0[i] * 100) + ' %    (' + str(entriesB0[i]) + '/' + str(total_entries_B0) + ')'
        # finally calculating the total effective efficiency
        tot_eff_eff_B0 = tot_eff_eff_B0 + event_fraction_B0[i] * (rvalueB0[i]) * (rvalueB0[i])

    overall_eff = tot_eff_eff_B0
    print '*'
    print '*       ____________________________________________________________                               *'
    print '*      |                                                            |                              *'
    print '*        B0-TAGGER     TOTAL EFFECTIVE EFFICIENCY: ' + '{:.3f}'.format(tot_eff_eff_B0 * 100) + ' %   '
    print '*      |____________________________________________________________|                              *'
    print '*                                                                                                  *'
    print '****************************************************************************************************'
    print '*                                                                                                  *'

    # not that imortant
    print '* ---------------------------------                                                                *'
    print '* Efficiency Determination - easiest way                                                           *'
    print '* ---------------------------------                                                                *'
    total_entries_B0 = histo_belleplotB0.GetEntries()
    total_entries_B0Bar = histo_belleplotB0bar.GetEntries()
    total_entries_B0_wrong = histo_m1.GetEntries()
    total_entries_B0Bar_wrong = histo_m2.GetEntries()
    total_entries = total_entries_B0 + total_entries_B0Bar
    total_entries_wrong = total_entries_B0_wrong + total_entries_B0Bar_wrong

    wrong_tag_fraction_B0 = total_entries_B0_wrong / (total_entries_B0)
    wrong_tag_fraction_B0Bar = total_entries_B0Bar_wrong / (total_entries_B0Bar)
    wrong_tag_fraction = total_entries_wrong / (total_entries)
    right_tag_fraction_B0 = 1 - 2 * wrong_tag_fraction_B0
    right_tag_fraction_B0Bar = 1 - 2 * wrong_tag_fraction_B0Bar
    right_tag_fraction = 1 - 2 * wrong_tag_fraction
    wrong_eff_B0 = right_tag_fraction_B0 * right_tag_fraction_B0
    wrong_eff_B0Bar = right_tag_fraction_B0Bar * right_tag_fraction_B0Bar
    wrong_eff = right_tag_fraction * right_tag_fraction

    """
    print ' wrong_tag_fraction for B0: ' + str(wrong_tag_fraction_B0)
    print ' right_tag_fraction for B0: ' + str(right_tag_fraction_B0)
    print ' wrong calculated eff B0: ' + str(wrong_eff_B0)
    print ' wrong_tag_fraction for B0Bar: ' + str(wrong_tag_fraction_B0Bar)
    print ' right_tag_fraction for B0Bar: ' + str(right_tag_fraction_B0Bar)
    print ' wrong calculated eff B0Bar: ' + str(wrong_eff_B0Bar)
    """
    print '*     wrong_tag_fraction for all:   ' + '{:.3f}'.format(wrong_tag_fraction * 100) + ' %                                                       *'
    print '*     right_tag_fraction for all:   ' + '{:.3f}'.format(right_tag_fraction * 100) + ' %                                                       *'
    print '*     wrong calculated eff all:     ' + '{:.3f}'.format(wrong_eff * 100) + ' %                                                       *'
    print '*                                                                                                  *'

    # write out the histograms
    histo_avr_r.Write('', ROOT.TObject.kOverwrite)
    histo_entries_per_bin.Write('', ROOT.TObject.kOverwrite)

    histo_Cnet_output_B0.Write('', ROOT.TObject.kOverwrite)
    histo_Cnet_output_B0bar.Write('', ROOT.TObject.kOverwrite)
    histo_belleplotB0.Write('', ROOT.TObject.kOverwrite)
    histo_belleplotB0bar.Write('', ROOT.TObject.kOverwrite)
    histo_calib_B0.Write('', ROOT.TObject.kOverwrite)
    histo_calib_B0bar.Write('', ROOT.TObject.kOverwrite)

    # produce a pdf
    ROOT.gStyle.SetOptStat(0)
    Canvas1 = ROOT.TCanvas('Bla', 'Final Output', 1200, 800)
    Canvas1.cd()  # activate
    histo_belleplotB0.SetFillColorAlpha(ROOT.kBlue, 0.2)
    histo_belleplotB0.SetFillStyle(1001)
    histo_belleplotB0.GetYaxis().SetLabelSize(0.03)
    histo_belleplotB0.GetYaxis().SetTitleOffset(1.2)
    histo_belleplotB0.SetLineColor(ROOT.kBlue)
    histo_belleplotB0bar.SetFillColorAlpha(ROOT.kRed, 1.0)
    histo_belleplotB0bar.SetFillStyle(3005)
    histo_belleplotB0bar.SetLineColor(ROOT.kRed)
    #SetLabelSize etc SetTitle

    histo_belleplotB0.SetTitle('Final output; (qr)-output ; Events')
    histo_belleplotB0.SetMinimum(0)
    histo_belleplotB0.SetMaximum(10000)
    histo_belleplotB0.Draw('hist')
    histo_belleplotB0bar.Draw('hist same')

    leg = ROOT.TLegend(0.75, 0.8, 0.9, 0.9)
    leg.AddEntry(histo_belleplotB0, "true B0")
    leg.AddEntry(histo_belleplotB0bar, "true B0bar")
    leg.Draw()

    Canvas1.Update()
    #IPython.embed()
    Canvas1.SaveAs(workingDirectory + '/''Belleplot_both.pdf')

# **********************************************
# DETERMINATION OF INDIVIDUAL EFFECTIVE EFFICIENCY
# **********************************************

# keep in mind:
# the individual efficiency is determind on basis of the combiner training.
# Whereas the efficiency is determined on basis of the final expert output.

rr_subsample = array('d', [0.0, 0.25, 0.5, 0.625, 0.75, 0.875, 1.0])
# individual categories
trackLevelParticles = [
    ('Electron', 'QrOfeROEIsRightClassElectronIsFromBElectron'),
    ('IntermediateElectron', 'QrOfeROEIsRightClassIntermediateElectronIsFromBIntermediateElectron'),
    ('Muon', 'QrOfmuROEIsRightClassMuonIsFromBMuon'),
    ('IntermediateMuon', 'QrOfmuROEIsRightClassIntermediateMuonIsFromBIntermediateMuon'),
    ('Kaon', 'QrOfKROEIsRightClassKaonIsFromBKaon'),
    ('SlowPion', 'QrOfpiROEIsRightClassSlowPionIsFromBSlowPion'),
    ('FastPion', 'QrOfpiROEIsRightClassFastPionIsFromBFastPion'),
    ('Lambda', 'QrOfLambda0ROEIsRightClassLambdaIsFromBLambda'),
]

# needs the B0Tagger.root-file from combiner teacher
if Belle2.FileSystem.findFile(workingDirectory + '/B0Tagger.root'):
    rootfile2 = ROOT.TFile(workingDirectory + '/B0Tagger.root', 'UPDATE')
    tree2 = rootfile2.Get('B0Tagger_tree')
    rootfile2.cd()

    print '****************** MEASURED EFFECTIVE EFFICIENCY FOR INDIVIDUAL CATEGORIES *************************'
    print '*                                                                                                  *'
    # input: Classifier input from event-level. Output of event-level is recalculated for input on combiner-level.
    # but is re-evaluated under combiner target. Signal is B0, background is B0Bar.

    for (category, categoryInput) in trackLevelParticles:
        # histogram of input variable (only signal) - not yet a probability! It's a classifier plot!
        hist_signal = ROOT.TH1F('Signal_' + category, 'Input Signal (B0)' + category + ' (binning 50)', 50, 0.0, 1.0)
        # histogram of input variable (only background) - not yet a probability! It's a classifier plot!
        hist_background = ROOT.TH1F('Background_' + category, 'Input Background (B0bar)' + category + ' (binning 50)', 50, 0.0, 1.0)

        # per definiton that input is not comparable to the network output, this has to be transformed.
        # probability output from 0 to 1 (corresponds to net output probability) -> calculation below
        hist_prob = ROOT.TH1F('Probability_' + category, 'Transformed to probability (' + category + ')', 50, 0.0, 1.0)
        # qr output from -1 to 1 -> transformation below
        hist_qr = ROOT.TH1F('QR_' + category, 'Transformed to qr (' + category + ')', 50, -1.0, 1.0)
        # histogram for abs(qr), i.e. this histogram contains the r-values -> transformation below
        # also used to get the number of entries, sorted into 6 bins
        hist_absqr = ROOT.TH1F('AbsQR_' + category, 'Abs(qr) (' + category + ')', 6, rr_subsample)
        # histogram contains at the end the average r values -> calculation below
        # sorted into 6 bins
        hist_aver_r = ROOT.TH1F('AverageR_' + category, 'A good one' + category, 6, rr_subsample)
        ###### TEST OF CALIBRATION ######
        ## for calibration plot we want to have
        hist_all = ROOT.TH1F('All_' + category, 'Input Signal (B0) and Background (B0Bar)' + category + ' (binning 50)', 50, 0.0, 1.0)
        tree2.Draw(categoryInput + '>>All_' + category)
        hist_calib_B0 = ROOT.TH1F('Calib_B0_' + category, 'Calibration Plot for true B0' + category + ' (binning 50)', 50, 0.0, 1.0)
        tree2.Draw(categoryInput + '>>Calib_B0_' + category, 'qr_Combined>0.5')
        hist_calib_B0.Divide(hist_all)

        # fill signal
        tree2.Draw(categoryInput + '>>Signal_' + category, 'qr_Combined>0.5')
        # fill background
        tree2.Draw(categoryInput + '>>Background_' + category, 'qr_Combined<0.5')

        ##### produce the input plots from combiner level #####

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
        hist_signal.SetLineColor(ROOT.kBlue)
        hist_background.SetFillColorAlpha(ROOT.kRed, 1.0)
        hist_background.SetFillStyle(3005)
        hist_background.SetLineColor(ROOT.kRed)

        hist_signal.SetTitle(category + ' category input; Network output ; Events')
        #hist_signal.SetMinimum(0)
        #hist_signal.SetMaximum(10000)

        hist_signal.Draw('hist')
        hist_background.Draw('hist same')

        l = ROOT.TLegend(0.6, 0.75, 0.8, 0.9)
        l.AddEntry(hist_signal, "true B0")
        l.AddEntry(hist_background, "true B0bar")
        l.SetTextSize(0.05)
        l.Draw()

        Canvas.Update()
        Canvas.SaveAs(workingDirectory + '/' + category + '_Input_Combiner.pdf')

        ###### TEST OF CALIBRATION ######

        # initialize some arrays
        purity = array('d', [0] * 51)
        purity2 = array('d', [0] * 51)
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
                purity[i] = 0
                purity2[i] = 0
            else:
                purity[i] = signal[i] / (signal[i] + back[i])
                purity2[i] = -1 + 2 * signal[i] / (signal[i] + back[i])

            # filling histogram with probabilty from 0 to 1
            hist_prob.Fill(purity[i], weight[i])
            # filling histogram with qr from -1 to 1
            hist_qr.Fill(purity2[i], weight[i])

            # filling histogram with abs(qr), i.e. this histogram contains the r-values (not qr)
            hist_absqr.Fill(abs(purity2[i]), weight[i])
            # filling histogram with abs(qr) special weighted - needed for average r calculation
            hist_aver_r.Fill(abs(purity2[i]), abs(purity2[i]) * weight[i])

        # hist_aver_r contains now the average r-value
        hist_aver_r.Divide(hist_absqr)

        # now calculating the efficiency

        # calculating number of events
        tot_entries = 0
        for i in range(1, 7):
            tot_entries = tot_entries + hist_absqr.GetBinContent(i)
        # initializing some arrays
        tot_eff_eff = 0
        event_fraction = array('f', [0] * 7)
        rvalue = array('f', [0] * 7)
        wvalue = array('f', [0] * 7)
        entries = array('f', [0] * 7)

        for i in range(1, 7):
            rvalue[i] = hist_aver_r.GetBinContent(i)
            wvalue[i] = (1 - rvalue[i]) / 2
            entries[i] = hist_absqr.GetBinContent(i)
            event_fraction[i] = entries[i] / tot_entries
            #print '*  Bin ' + str(i) + ' r-value: ' + str(rvalue[i]), 'entries: ' + str(event_fraction[i] * 100) + ' % (' + str(entries[i]) + '/' + str(tot_entries) + ')'
            tot_eff_eff = tot_eff_eff + event_fraction[i] * (rvalue[i]) * (rvalue[i])

        print '*    ' + '{: > 8.3f}'.format(tot_eff_eff * 100) + ' %' + '{:>85}'.format(category + '                     *')

        hist_signal.Write('', ROOT.TObject.kOverwrite)
        hist_background.Write('', ROOT.TObject.kOverwrite)
        hist_prob.Write('', ROOT.TObject.kOverwrite)
        hist_qr.Write('', ROOT.TObject.kOverwrite)
        hist_absqr.Write('', ROOT.TObject.kOverwrite)
        hist_aver_r.Write('', ROOT.TObject.kOverwrite)
        hist_all.Write('', ROOT.TObject.kOverwrite)
        hist_calib_B0.Write('', ROOT.TObject.kOverwrite)

    if overall_eff != 0:
        print '*    -------------------------------------------------------------------------'
        print '*    ' + '{: > 8.3f}'.format(overall_eff * 100) + ' %' + '{:>85}'.format('TOTAL' + '                     *')

    print '*                                                                                                  *'
    print '****************************************************************************************************'

    # close root-file and goodbye
    rootfile2.Close()
else:
    B2INFO("Some error. Sorry. Please look into code.")
