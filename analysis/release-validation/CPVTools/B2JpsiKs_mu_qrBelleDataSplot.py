#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# * **********************************  Flavor Tagging   ************************************
#                                                                                          *
# * This validation script performs an Splot analysis of Belle Data using mbc as           *
# * Splot variable in order to extract the qr distribution of signal B0s.                  *
# * The signal component of Mbc is first fitted on converted Belle MC using a              *
# * 2 Gaussian function. The Mbc component is used together with an Argus function         *
# * to fit the mbc distribution of converted Belle data.                                   *
# * An Splot is performed and its output variable used to weigth the qr                    *
# * distribution.                                                                          *
# * For more information see Sec. 4.8 in BELLE2-PTHESIS-2018-003                           *
# * Usage:                                                                                 *
# *   basf2 B2JpsiKs_mu_qrBelleDataSplot.py RootNtupleFilesMC RootNtupleFilesData treeName *
#                                                                                          *
# * Contributors: F. Abudinen (December 2018)                                              *
#                                                                                          *
# ******************************************************************************************

import math
import glob
import sys
from array import array
import matplotlib.pyplot as plt
from defaultEvaluationParameters import r_size, r_subsample, rbins
import ROOT
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams['text.usetex'] = True


PATH = "."

workingFilesMC = str(sys.argv[1])
workingFilesData = str(sys.argv[2])

workingFilesMC = glob.glob(str(workingFilesMC))
workingFilesData = glob.glob(str(workingFilesData))

lim = 10
limZ = 0.03
limZsig = 0.05

treenames = [str(sys.argv[3])]

mc_total_notTagged = 0

data_total_notTagged = 0

data_total_Tagged = 0

ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)


def efficiencyCalculator(mc, data, method):

    # Calculate Efficiency for MC

    mc_dataB0 = mc[np.where(mc[:, 1] > 0)]
    mc_dataB0bar = mc[np.where(mc[:, 1] < 0)]

    mc_totaldataB0 = np.absolute(mc_dataB0[:, 0])
    mc_totaldataB0bar = np.absolute(mc_dataB0bar[:, 0])

    mc_rvalueB0 = (np.histogram(mc_totaldataB0, rbins, weights=mc_totaldataB0)[0] / np.histogram(mc_totaldataB0, rbins)[0])
    mc_rvalueMSB0 = (
        np.histogram(
            mc_totaldataB0,
            rbins,
            weights=mc_totaldataB0 *
            mc_totaldataB0)[0] /
        np.histogram(
            mc_totaldataB0,
            rbins)[0])
    mc_rvalueStdB0 = np.sqrt(abs(mc_rvalueMSB0 - mc_rvalueB0 * mc_rvalueB0))
    mc_entriesB0 = np.histogram(mc_totaldataB0, rbins)[0]

    mc_rvalueB0bar = (
        np.histogram(
            mc_totaldataB0bar,
            rbins,
            weights=mc_totaldataB0bar)[0] /
        np.histogram(
            mc_totaldataB0bar,
            rbins)[0])
    mc_rvalueMSB0bar = (
        np.histogram(
            mc_totaldataB0bar,
            rbins,
            weights=mc_totaldataB0bar *
            mc_totaldataB0bar)[0] /
        np.histogram(
            mc_totaldataB0bar,
            rbins)[0])
    mc_rvalueStdB0bar = np.sqrt(abs(mc_rvalueMSB0bar - mc_rvalueB0bar * mc_rvalueB0bar))
    mc_entriesB0bar = np.histogram(mc_totaldataB0bar, rbins)[0]

    mc_total_entriesB0 = mc_totaldataB0.shape[0] + mc_total_notTagged / 2
    mc_total_tagged_B0 = mc_totaldataB0.shape[0]
    mc_event_fractionB0 = mc_entriesB0.astype(float) / mc_total_entriesB0

    mc_total_entriesB0bar = mc_totaldataB0bar.shape[0] + mc_total_notTagged / 2
    mc_total_tagged_B0bar = mc_totaldataB0bar.shape[0]
    mc_event_fractionB0bar = mc_entriesB0bar.astype(float) / mc_total_entriesB0bar

    mc_total_entries = mc_total_entriesB0 + mc_total_entriesB0bar
    mc_total_tagged = mc_totaldataB0.shape[0] + mc_totaldataB0bar.shape[0]
    mc_event_fractionTotal = (mc_entriesB0.astype(float) + mc_entriesB0bar.astype(float)) / mc_total_entries

    mc_tagging_eff = mc_total_tagged / (mc_total_tagged + mc_total_notTagged)
    mc_DeltaTagging_eff = math.sqrt(mc_total_tagged * mc_total_notTagged**2 +
                                    mc_total_notTagged * mc_total_tagged**2) / (mc_total_entries**2)
    mc_tot_eff_effB0 = 0
    mc_tot_eff_effB0bar = 0
    mc_uncertainty_eff_effB0 = 0
    mc_uncertainty_eff_effB0bar = 0
    mc_uncertainty_eff_effAverage = 0
    mc_diff_eff_Uncertainty = 0
    mc_event_fractionDiff = array('f', [0] * r_size)
    mc_rvalueB0Average = array('f', [0] * r_size)
    mc_rvalueStdB0Average = array('f', [0] * r_size)
    mc_wvalue = array('f', [0] * r_size)
    mc_wvalueUncertainty = array('f', [0] * r_size)
    mc_wvalueB0 = array('f', [0] * r_size)
    mc_wvalueB0bar = array('f', [0] * r_size)
    mc_wvalueDiff = array('f', [0] * r_size)
    mc_wvalueDiffUncertainty = array('f', [0] * r_size)
    mc_iEffEfficiency = array('f', [0] * r_size)
    mc_iEffEfficiencyUncertainty = array('f', [0] * r_size)
    mc_iEffEfficiencyB0Uncertainty = array('f', [0] * r_size)
    mc_iEffEfficiencyB0barUncertainty = array('f', [0] * r_size)
    mc_iDeltaEffEfficiency = array('f', [0] * r_size)
    mc_iDeltaEffEfficiencyUncertainty = array('f', [0] * r_size)

    print('****************** MEASURED EFFECTIVE EFFICIENCY FOR COMBINER USING ' + method + ' ***************************')
    print('*                                                                                                  *')
    print('*                 -->  DETERMINATION BASED ON MONTE CARLO                                          *')
    print('*                                                                                                  *')
    print('* ------------------------------------------------------------------------------------------------ *')
    print('*   r-interval          <r>        Efficiency   Delta_Effcy         w               Delta_w        *')
    print('* ------------------------------------------------------------------------------------------------ *')

    for i in range(0, r_size - 1):
        mc_rvalueB0Average[i] = (mc_rvalueB0[i] + mc_rvalueB0bar[i]) / 2
        mc_rvalueStdB0Average[i] = math.sqrt(mc_rvalueStdB0[i]**2 / (mc_entriesB0[0] - 1) +
                                             mc_rvalueStdB0bar[i]**2 / (mc_entriesB0bar[0] - 1)) / 2
        mc_wvalueB0[i] = (1 - mc_rvalueB0[i]) / 2
        mc_wvalueB0bar[i] = (1 - mc_rvalueB0bar[i]) / 2
        mc_wvalueDiff[i] = mc_wvalueB0[i] - mc_wvalueB0bar[i]
        mc_wvalueDiffUncertainty[i] = math.sqrt((mc_rvalueStdB0[i] / 2)**2 / (mc_entriesB0[0] - 1) +
                                                (mc_rvalueStdB0bar[i] / 2)**2 / (mc_entriesB0bar[0] - 1))
        mc_wvalue[i] = (mc_wvalueB0[i] + mc_wvalueB0bar[i]) / 2
        mc_wvalueUncertainty[i] = mc_wvalueDiffUncertainty[i] / 2
        # fraction of events/all events

        mc_event_fractionDiff[i] = (mc_entriesB0[i] - mc_entriesB0bar[i]) / mc_total_entries

        mc_iEffEfficiency[i] = mc_tagging_eff * (mc_event_fractionB0[i] * mc_rvalueB0[i] * mc_rvalueB0[i] +
                                                 mc_event_fractionB0bar[i] * mc_rvalueB0bar[i] * mc_rvalueB0bar[i]) / 2

        mc_iEffEfficiencyB0Uncertainty[i] = mc_rvalueB0[i] * \
            math.sqrt((2 * mc_total_entriesB0 * mc_entriesB0[i] * mc_rvalueStdB0[i])**2 / (mc_entriesB0[0] - 1) +
                      mc_rvalueB0[i]**2 * mc_entriesB0[i] *
                      (mc_total_entriesB0 * (mc_total_entriesB0 - mc_entriesB0[i]) +
                       mc_entriesB0[i] * mc_total_notTagged)) / (mc_total_entriesB0**2)
        mc_iEffEfficiencyB0barUncertainty[i] = mc_rvalueB0bar[i] * \
            math.sqrt((2 * mc_total_entriesB0bar * mc_entriesB0bar[i] * mc_rvalueStdB0bar[i])**2 / (mc_entriesB0bar[0] - 1) +
                      mc_rvalueB0bar[i]**2 * mc_entriesB0bar[i] *
                      (mc_total_entriesB0bar * (mc_total_entriesB0bar - mc_entriesB0bar[i]) +
                       mc_entriesB0bar[i] * mc_total_notTagged)) / (mc_total_entriesB0bar**2)

        mc_iDeltaEffEfficiency[i] = mc_event_fractionB0[i] * mc_rvalueB0[i] * \
            mc_rvalueB0[i] - mc_event_fractionB0bar[i] * mc_rvalueB0bar[i] * mc_rvalueB0bar[i]

        mc_iDeltaEffEfficiencyUncertainty[i] = math.sqrt(
            mc_iEffEfficiencyB0Uncertainty[i]**2 +
            mc_iEffEfficiencyB0barUncertainty[i]**2)

        mc_iEffEfficiencyUncertainty[i] = mc_iDeltaEffEfficiencyUncertainty[i] / 2

        mc_diff_eff_Uncertainty = mc_diff_eff_Uncertainty + mc_iDeltaEffEfficiencyUncertainty[i]**2

        # finally calculating the total effective efficiency
        mc_tot_eff_effB0 = mc_tot_eff_effB0 + mc_event_fractionB0[i] * mc_rvalueB0[i] * mc_rvalueB0[i]
        mc_tot_eff_effB0bar = mc_tot_eff_effB0bar + mc_event_fractionB0bar[i] * mc_rvalueB0bar[i] * mc_rvalueB0bar[i]
        mc_uncertainty_eff_effAverage = mc_uncertainty_eff_effAverage + mc_iEffEfficiencyUncertainty[i]**2
        mc_uncertainty_eff_effB0 = mc_uncertainty_eff_effB0 + mc_iEffEfficiencyB0Uncertainty[i]**2
        mc_uncertainty_eff_effB0bar = mc_uncertainty_eff_effB0bar + mc_iEffEfficiencyB0barUncertainty[i]**2

        # intervallEff[i] = event_fractionTotal[i] * rvalueB0Average[i] * rvalueB0Average[i]
        print('* ' + '{:.3f}'.format(r_subsample[i]) + ' - ' + '{:.3f}'.format(r_subsample[i + 1]) + '   ' +
              '{:.3f}'.format(mc_rvalueB0Average[i]) + ' +- ' + '{:.4f}'.format(mc_rvalueStdB0Average[i]) + '    ' +
              '{:.4f}'.format(mc_event_fractionTotal[i]) + '      ' +
              '{: .4f}'.format(mc_event_fractionDiff[i]) + '     ' +
              '{:.4f}'.format(mc_wvalue[i]) + ' +- ' + '{:.4f}'.format(mc_wvalueUncertainty[i]) + '   ' +
              '{: .4f}'.format(mc_wvalueDiff[i]) + ' +- ' + '{:.4f}'.format(mc_wvalueDiffUncertainty[i]) + '  *')

    mc_average_eff_eff = (mc_tot_eff_effB0 + mc_tot_eff_effB0bar) / 2
    mc_uncertainty_eff_effAverage = math.sqrt(mc_uncertainty_eff_effAverage)
    mc_uncertainty_eff_effB0 = math.sqrt(mc_uncertainty_eff_effB0)
    mc_uncertainty_eff_effB0bar = math.sqrt(mc_uncertainty_eff_effB0bar)
    mc_diff_eff = mc_tot_eff_effB0 - mc_tot_eff_effB0bar
    mc_diff_eff_Uncertainty = math.sqrt(mc_diff_eff_Uncertainty)

    print('* ------------------------------------------------------------------------------------------------ *')
    print('*                                                                                                  *')
    print('*    __________________________________________________________________________________________    *')
    print('*   |                                                                                          |   *')
    print('*   | TOTAL NUMBER OF TAGGED EVENTS  =  ' +
          '{:<24}'.format("%.0f" % mc_total_tagged) + '{:>36}'.format('|   *'))
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFICIENCY  (q=+-1)=  ' +
        '{:.2f}'.format(
            mc_tagging_eff *
            100) +
        " +- " +
        '{:.2f}'.format(
            mc_DeltaTagging_eff *
            100) +
        ' %                                      |   *')
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY  (q=+-1)=  ' +
        '{:.6f}'.format(
            mc_average_eff_eff *
            100) +
        " +- " +
        '{:.6f}'.format(
            mc_uncertainty_eff_effAverage *
            100) +
        ' %                    |   *')
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY ASYMMETRY (q=+-1)=  ' +
        '{:^9.6f}'.format(
            mc_diff_eff *
            100) +
        " +- " +
        '{:.6f}'.format(
            mc_diff_eff_Uncertainty *
            100) +
        ' %           |   *')
    print('*   |                                                                                          |   *')
    print('*   | B0-TAGGER  TOTAL EFFECTIVE EFFICIENCIES: ' +
          '{:.2f}'.format(mc_tot_eff_effB0 * 100) + " +-" + '{: 4.2f}'.format(mc_uncertainty_eff_effB0 * 100) +
          ' % (q=+1)  ' +
          '{:.2f}'.format(mc_tot_eff_effB0bar * 100) + " +-" + '{: 4.2f}'.format(mc_uncertainty_eff_effB0bar * 100) +
          ' % (q=-1) ' + ' |   *')
    print('*   |                                                                                          |   *')
    print('*   | FLAVOR PERCENTAGE (MC):                  ' +
          '{:.2f}'.format(mc_total_tagged_B0 / mc_total_tagged * 100) + ' % (q=+1)  ' +
          '{:.2f}'.format(mc_total_tagged_B0bar / mc_total_tagged * 100) + ' % (q=-1)  Diff=' +
          '{:^5.2f}'.format((mc_total_tagged_B0 - mc_total_tagged_B0bar) / mc_total_tagged * 100) + ' %    |   *')
    print('*   |__________________________________________________________________________________________|   *')
    print('*                                                                                                  *')
    print('****************************************************************************************************')

    print(r'\begin{tabular}{ l  r  r  r  r  r  r  r }')
    print(r'\hline')
    print(r'$r$- Interval & $\varepsilon_i\ $ & $w_i \pm \delta w_i\enskip\, $ ' +
          r' & $\Delta w_i \pm \delta\Delta w_i $& $\varepsilon_{\text{eff}, i} \pm \delta\varepsilon_{\text{eff}, i}\enskip$ ' +
          r' & & & $\Delta \varepsilon_{\text{eff}, i}  \pm \delta\Delta \varepsilon_{\text{eff}, i} $\\ \hline\hline')
    for i in range(0, r_size - 1):
        print('$ ' + '{:.3f}'.format(r_subsample[i]) + ' - ' + '{:.3f}'.format(r_subsample[i + 1]) + '$ & $'
              '{: 6.1f}'.format(mc_event_fractionTotal[i] * 100) + '$ & $' +
              '{: 7.3f}'.format(mc_wvalue[i] * 100) + r" \pm " + '{:2.3f}'.format(mc_wvalueUncertainty[i] * 100) + r' $ & $' +
              '{: 6.1f}'.format(mc_wvalueDiff[i] * 100) + r" \pm " +
              '{:2.3f}'.format(mc_wvalueDiffUncertainty[i] * 100) + r'\, $ & $' +
              '{: 8.4f}'.format(mc_iEffEfficiency[i] * 100) +  # + '$ & $' +
              r" \pm " + '{:2.4f}'.format(mc_iEffEfficiencyUncertainty[i] * 100) + r'\, $ & & & $' +
              '{: 6.4f}'.format(mc_iDeltaEffEfficiency[i] * 100) +  # +
              r" \pm " + '{:2.4f}'.format(mc_iDeltaEffEfficiencyUncertainty[i] * 100) +
              r'\enskip $ \\ ')
    print(r'\hline\hline')
    print(
        r'\multicolumn{1}{r}{Total} &  & \multicolumn{3}{r}{ $\varepsilon_\text{eff} = ' +
        r'\sum_i \varepsilon_i \cdot \langle 1-2w_i\rangle^2 = ' +
        '{: 6.2f}'.format(
            mc_average_eff_eff *
            100) +
        r" \pm " +
        '{: 6.2f}'.format(
            mc_uncertainty_eff_effAverage *
            100) +
        r'\quad\,$ }')
    print(r'& & \multicolumn{2}{r}{ $\Delta \varepsilon_\text{eff} = ' +
          '{: 6.2f}'.format(mc_diff_eff * 100) + r" \pm " + '{: 6.2f}'.format(mc_diff_eff_Uncertainty * 100) + r'\ \quad\,  $ }' +
          r' \\')
    print(r'\hline')
    print(r'\end{tabular}')

    # Calculate Efficiency for Data

    data_totaldata = np.absolute(data[:, 0])
    data_splotWeights = data[:, 1]

    data_entries = np.histogram(data_totaldata, rbins, weights=data_splotWeights)[0]
    data_rvalueB0Average = (np.histogram(data_totaldata, rbins, weights=data_totaldata * data_splotWeights)[0] / data_entries)
    data_rvalueMSB0Average = (
        np.histogram(
            data_totaldata,
            rbins,
            weights=(data_totaldata)**2 *
            data_splotWeights)[0] /
        data_entries)
    data_rvalueStdB0Average = np.sqrt(abs(data_rvalueMSB0Average - data_rvalueB0Average * data_rvalueB0Average))

    data_total_tagged = 0
    for iEntries in data_entries:
        data_total_tagged += iEntries

    data_total_notTaggedWeighted = data_total_notTagged * data_total_tagged / data_totaldata.shape[0]

    data_total_entries = data_total_tagged + data_total_notTaggedWeighted
    data_event_fractionTotal = data_entries.astype(float) / data_total_entries

    data_tagging_eff = data_total_tagged / data_total_entries
    data_DeltaTagging_eff = math.sqrt(data_total_tagged * data_total_notTaggedWeighted**2 +
                                      data_total_notTaggedWeighted * data_total_tagged**2) / (data_total_entries**2)
    data_average_eff_eff = 0
    data_uncertainty_eff_effAverage = 0
    data_wvalue = array('f', [0] * r_size)
    data_wvalueUncertainty = array('f', [0] * r_size)
    data_iEffEfficiency = array('f', [0] * r_size)
    data_iEffEfficiencyUncertainty = array('f', [0] * r_size)

    print('****************** MEASURED EFFECTIVE EFFICIENCY FOR COMBINER USING ' + method + ' ***************************')
    print('*                                                                                                  *')
    print('*                 -->  DETERMINATION BASED ON DATA                                                 *')
    print('*                                                                                                  *')
    print('* ------------------------------------------------------------------------------------------------ *')
    print('*   r-interval             <r>               Efficiency                    w                       *')
    print('* ------------------------------------------------------------------------------------------------ *')

    for i in range(0, r_size - 1):
        data_wvalue[i] = (1 - data_rvalueB0Average[i]) / 2
        data_wvalueUncertainty[i] = (data_rvalueStdB0Average[i] / math.sqrt(data_entries[i] - 1)) / 2
        # fraction of events/all events

        data_iEffEfficiency[i] = data_tagging_eff * (data_event_fractionTotal[i] *
                                                     data_rvalueB0Average[i] * data_rvalueB0Average[i])

        data_iEffEfficiencyUncertainty[i] = data_rvalueB0Average[i] * \
            math.sqrt((2 * data_total_entries * data_entries[i] *
                       (data_rvalueStdB0Average[i] / math.sqrt(data_entries[i] - 1)))**2 +
                      data_rvalueB0Average[i]**2 * data_entries[i] *
                      (data_total_entries * (data_total_entries - data_entries[i]) +
                       data_entries[i] * data_total_notTaggedWeighted)) / (data_total_entries**2)

        # finally calculating the total effective efficiency
        data_average_eff_eff = data_average_eff_eff + \
            data_event_fractionTotal[i] * data_rvalueB0Average[i] * data_rvalueB0Average[i]
        data_uncertainty_eff_effAverage = data_uncertainty_eff_effAverage + data_iEffEfficiencyUncertainty[i]**2

        # intervallEff[i] = event_fractionTotal[i] * rvalueB0Average[i] * rvalueB0Average[i]
        print('* ' + '{:.3f}'.format(r_subsample[i]) + ' - ' + '{:.3f}'.format(r_subsample[i + 1]) + '       ' +
              '{:.3f}'.format(data_rvalueB0Average[i]) + ' +- ' + '{:.4f}'.format(data_rvalueStdB0Average[i]) + '          ' +
              '{:.4f}'.format(data_event_fractionTotal[i]) + '               ' +
              '{:.4f}'.format(data_wvalue[i]) + ' +- ' + '{:.4f}'.format(data_wvalueUncertainty[i]) + '             ' + '  *')

    data_uncertainty_eff_effAverage = math.sqrt(data_uncertainty_eff_effAverage)

    print('* ------------------------------------------------------------------------------------------------ *')
    print('*                                                                                                  *')
    print('*    __________________________________________________________________________________________    *')
    print('*   |                                                                                          |   *')
    print('*   | TOTAL NUMBER OF TAGGED EVENTS  =  ' +
          '{:<24}'.format("%.0f" % data_total_tagged) + '{:>36}'.format('|   *'))
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFICIENCY  (q=+-1)=  ' +
        '{:.2f}'.format(
            data_tagging_eff *
            100) +
        " +- " +
        '{:.2f}'.format(
            data_DeltaTagging_eff *
            100) +
        ' %                                      |   *')
    print('*   |                                                                                          |   *')
    print(
        '*   | TOTAL AVERAGE EFFECTIVE EFFICIENCY  (q=+-1)=  ' +
        '{:.6f}'.format(
            data_average_eff_eff *
            100) +
        " +- " +
        '{:.6f}'.format(
            data_uncertainty_eff_effAverage *
            100) +
        ' %                    |   *')
    print('*   |__________________________________________________________________________________________|   *')
    print('*                                                                                                  *')
    print('****************************************************************************************************')

    print(r'\begin{tabular}{ l  r  r  r  r}')
    print(r'\hline')
    print(r'$r$- Interval & $\varepsilon_i\ $ & $w_i \pm \delta w_i\ $ ' +
          r' & $\varepsilon_{\text{eff}, i} \pm \delta\varepsilon_{\text{eff}, i}$ ' +
          r'\\ \hline\hline')
    for i in range(0, len(rbins) - 1):
        print('$ ' + '{:.3f}'.format(r_subsample[i]) + ' - ' + '{:.3f}'.format(r_subsample[i + 1]) + '$ & $'
              '{: 6.2f}'.format(data_event_fractionTotal[i] * 100) + '$ & $' +
              '{: 6.2f}'.format(data_wvalue[i] * 100) + r" \pm " + '{:2.2f}'.format(data_wvalueUncertainty[i] * 100) + r'\, $ & $' +
              '{: 7.3f}'.format(data_iEffEfficiency[i] * 100) +  # + '$ & $' +
              r" \pm " + '{:4.3f}'.format(data_iEffEfficiencyUncertainty[i] * 100) + r'\, $ \\ ')
    print(r'\hline\hline')
    print(
        r'\multicolumn{1}{r}{Total} &  \multicolumn{3}{r}{ $\varepsilon_\text{eff} = ' +
        r'\sum_i \varepsilon_i \cdot \langle 1-2w_i\rangle^2 = ' +
        '{: 6.1f}'.format(
            data_average_eff_eff *
            100) +
        r" \pm " +
        '{: 6.1f}'.format(
            data_uncertainty_eff_effAverage *
            100) +
        r'\quad\,$ }' +
        r' \\')
    print(r'\hline')
    print(r'\end{tabular}')

    # Plot of Signal for B2TIP

    # -----bins
    bins = list(range(-25, 26, 1))
    for i in range(0, len(bins)):
        bins[i] = float(bins[i]) / 25
    # ------

    fig1 = plt.figure(1, figsize=(11, 8))
    ax1 = plt.axes([0.21, 0.15, 0.76, 0.8])

    qrDataPoints, qrDataBins = np.histogram(data[:, 0], bins=bins, weights=data_splotWeights)
    qrDataPointsStd = np.sqrt(qrDataPoints)

    qrDataPointsNorm, qrDataBins = np.histogram(data[:, 0], bins=bins, weights=data_splotWeights, density=1)
    qrDataPointsStdNorm = qrDataPointsStd * qrDataPointsNorm / qrDataPoints

    qrBincenters = 0.5 * (qrDataBins[1:] + qrDataBins[:-1])

    p3 = ax1.errorbar(qrBincenters, qrDataPointsNorm, xerr=0.02, yerr=qrDataPointsStdNorm, elinewidth=2, mew=2, ecolor='k',
                      fmt='o', mfc='k', markersize=6, label=r'${\rm Data}$')

    ax1.hist(mc[:, 0], bins=bins, density=1, histtype='step',
             edgecolor='b', linewidth=4, linestyle='dashed', label=r'${\rm MC}$')

    p2, =  ax1.plot([], label=r'${\rm MC}$', linewidth=4, linestyle='dashed', c='b')

    methodLabel = method
    if method == "FANN":
        methodLabel = "MLP"

    ax1.set_ylabel(r'${\rm Fraction\ \ of\ \ Events\ /\ (\, 0.04\, )}$', fontsize=35)
    ax1.set_xlabel(r'$(q\cdot r)_{\rm ' + methodLabel + '}$', fontsize=35)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=40)
    ax1.tick_params(axis='y', labelsize=35)
    ax1.legend([p3, p2], [r'${\rm Data}$', r'${\rm MC}$'], prop={'size': 35}, loc=0, numpoints=1)
    ax1.grid(True)

    if qrDataPointsNorm[np.argmax(qrDataPointsNorm)] < 1.4:
        ax1.set_ylim(0, 1.4)

    ax1.set_xlim(-1.005, 1.005)
    plt.savefig(PATH + '/QR_B0_B0bar' + methodLabel + '.pdf')
    fig1.clear()

    # Plot of Continuum for B2TIP and Thesis

    fig2 = plt.figure(2, figsize=(11, 8))
    ax1 = plt.axes([0.21, 0.15, 0.76, 0.8])

    qrDataPointsCont, qrDataBins = np.histogram(data[:, 0], bins=bins, weights=data[:, 2])
    qrDataPointsContStd = np.sqrt(qrDataPointsCont)

    qrDataPointsContNorm, qrDataBins = np.histogram(data[:, 0], bins=bins, weights=data[:, 2], density=1)
    qrDataPointsContStdNorm = qrDataPointsContStd * qrDataPointsContNorm / qrDataPointsCont

    qrBincenters = 0.5 * (qrDataBins[1:] + qrDataBins[:-1])

    p3 = ax1.errorbar(qrBincenters, qrDataPointsContNorm, xerr=0.02, yerr=qrDataPointsContStdNorm, elinewidth=2, mew=2, ecolor='k',
                      fmt='o', mfc='k', mec='#990000', markersize=6, label=r'${\rm Data}$')

    ax1.set_ylabel(r'${\rm Fraction\ \ of\ \ Events\ /\ (\, 0.04\, )}$', fontsize=35)
    ax1.set_xlabel(r'$(q\cdot r)_{\rm ' + methodLabel + '}$', fontsize=35)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=40)
    ax1.tick_params(axis='y', labelsize=35)
    ax1.legend([p3], [r'${\rm Data}$'], prop={'size': 35}, loc=1, numpoints=1)
    ax1.grid(True)

    if qrDataPointsContNorm[np.argmax(qrDataPointsContNorm)] < 1.4:
        ax1.set_ylim(0, 1.4)

    ax1.set_xlim(-1.005, 1.005)
    plt.savefig(PATH + '/QR_B0_B0bar' + methodLabel + 'Continuum.pdf')
    fig2.clear()

    data_total_tagged_Continuum = 0
    for iEntries in qrDataPointsCont:
        data_total_tagged_Continuum += iEntries

    print('****************************************************************************************************')
    print('*   TOTAL NUMBER OF TAGGED CONTINUUM EVENTS  =  ' +
          '{:<24}'.format("%.0f" % data_total_tagged_Continuum) + '{:>28}'.format(' *'))

    # Chi-squared between MC and Data

    mcHistW = ROOT.TH1F("mcHistW", "mcHistW", int(r_size - 2), r_subsample)
    dataHistW = ROOT.TH1F("dataHistW", "dataHistW", int(r_size - 2), r_subsample)

    mcHistEff = ROOT.TH1F("mcHistEff", "mcHistEff", int(r_size - 2), r_subsample)
    dataHistEff = ROOT.TH1F("dataHistEff", "dataHistEff", int(r_size - 2), r_subsample)

    print('****************************************************************************************************')
    print('*                                                                                                  *')
    print('*                 -->  TEST OF COMPATIBILITY BETWEEN DATA AND MC                                   *')

    for i in range(0, len(rbins) - 1):

        mcHistW.SetBinContent(i + 1, mc_wvalue[i])
        mcHistW.SetBinError(i + 1, mc_wvalueUncertainty[i])

        dataHistW.SetBinContent(i + 1, data_wvalue[i])
        dataHistW.SetBinError(i + 1, data_wvalueUncertainty[i])

        mcHistEff.SetBinContent(i + 1, mc_iEffEfficiency[i])
        mcHistEff.SetBinError(i + 1, mc_iEffEfficiencyUncertainty[i])

        dataHistEff.SetBinContent(i + 1, data_iEffEfficiency[i])
        dataHistEff.SetBinError(i + 1, data_iEffEfficiencyUncertainty[i])

    residualsW = array('d', [0] * r_size)
    residualsEff = array('d', [0] * r_size)
    print('*                                                                                                  *')
    print('*      TEST ON w                                                                                   *')
    print('*                                                                                                  *')

    chi2W = dataHistW.Chi2Test(mcHistW, "WW CHI2 P", residualsW)
    print("1-CL for NDF = 7 is = ", ROOT.TMath.Prob(chi2W, 7))
    print('*                                                                                                  *')
    print('*      TEST ON EFFECTIVE EFFICIENCY                                                                *')
    print('*                                                                                                  *')
    chi2Eff = dataHistEff.Chi2Test(mcHistEff, "WW P", residualsEff)
    print("1-CL for NDF = 7 is = ", ROOT.TMath.Prob(chi2Eff, 7))

    print('****************************************************************************************************')

    # Plot of Signal with Residuals for Thesis

    qrMCPoints, qrDataBins = np.histogram(mc[:, 0], bins=bins)
    qrMCPointsStd = np.sqrt(qrMCPoints)
    qrMCPointsNorm, qrDataBins = np.histogram(mc[:, 0], bins=bins, density=1)
    qrMCPointsStdNorm = qrMCPointsStd * qrMCPointsNorm / qrMCPoints

    qrDataMCResiduals = (qrDataPointsNorm - qrMCPointsNorm) \
        / np.sqrt(qrDataPointsStdNorm * qrDataPointsStdNorm + qrMCPointsStdNorm * qrMCPointsStdNorm)

    fig3 = plt.figure(3, figsize=(11, 11))
    ax1 = plt.axes([0.21, 0.37, 0.76, 0.60])
    # print('Here Log Scale')
    # ax1.set_yscale('log', nonposy='clip') # Only for Semileptonic
    p3 = ax1.errorbar(qrBincenters, qrDataPointsNorm, xerr=0.02, yerr=qrDataPointsStdNorm,
                      elinewidth=2, mew=2, ecolor='k',
                      fmt='o', mfc='k', mec='#006600', markersize=6, label=r'${\rm Data}$')

    ax1.hist(mc[:, 0], bins=bins, density=1, histtype='step',
             edgecolor='b', linewidth=4, linestyle='dashed', label=r'${\rm MC}$')

    p2, =  ax1.plot([], label=r'${\rm MC}$', linewidth=4, linestyle='dashed', c='b')

    ax1.set_ylabel(r'${\rm Fraction\ \ of\ \ Events\ /\ (\, 0.04\, )}$', fontsize=35, labelpad=20)
    # ax1.set_xlabel(r'', fontsize=35)
    ax1.tick_params(axis='y', labelsize=35)
    ax1.legend([p3, p2], [r'${\rm Data}$', r'${\rm MC}$'], prop={'size': 35}, loc=1, numpoints=1)
    ax1.grid(True)

    if qrDataPointsNorm[np.argmax(qrDataPointsNorm)] < 1.4:
        ax1.set_ylim(0, 1.4)
        plt.yticks([0, 0.25, 0.5, 0.75, 1.0, 1.25],
                   [r'$0.00$', r'$0.25$', r'$0.5$', r'$0.75$', r'$1.00$', r'$1.25$'], rotation=0, size=35)
    ax1.set_xlim(-1.005, 1.005)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'', r'', r'', r'', r'', r'', r'', r'', r''], rotation=0, size=35)

    ax2 = plt.axes([0.21, 0.15, 0.76, 0.2])
    ax2.errorbar(qrBincenters, qrDataMCResiduals, xerr=0.02, yerr=1, elinewidth=2, mew=2, ecolor='k',
                 fmt='o', mfc='k', mec='#006600', markersize=6, label=r'${\rm Data}$')
    ax2.set_ylabel(r'${\rm Normalized}$' + '\n' + r'${\rm Residuals}$', fontsize=35, labelpad=20)
    ax2.set_xlabel(r'$(q\cdot r)_{\rm ' + methodLabel + '}$', fontsize=35)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=35)
    plt.yticks([-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5],
               [r'', r'$-4$', r'', r'$-2$', r'', r'$0$', r'', r'$2$', r'', r'$4$', r''], rotation=0, size=25)
    ax2.set_ylim(-5, 5)
    # ax2.tick_params(axis='y', labelsize=30)
    ax2.set_xlim(-1.005, 1.005)
    ax2.xaxis.grid(True)  # linestyle='--'
    # plt.axhline(y= 4, xmin=-1.005, xmax=1.005, linewidth=1, color = 'k', linestyle = '-')
    plt.axhline(y=3, xmin=-1.005, xmax=1.005, linewidth=2, color='#006600', linestyle='-')
    # plt.axhline(y= 2, xmin=-1.005, xmax=1.005, linewidth=1, color = 'k', linestyle = '-')
    plt.axhline(y=1, xmin=-1.005, xmax=1.005, linewidth=2, color='b', linestyle='-')
    plt.axhline(y=-1, xmin=-1.005, xmax=1.005, linewidth=2, color='b', linestyle='-')
    # plt.axhline(y=-2, xmin=-1.005, xmax=1.005, linewidth=1, color = 'k', linestyle = '-')
    plt.axhline(y=-3, xmin=-1.005, xmax=1.005, linewidth=2, color='#006600', linestyle='-')
    # plt.axhline(y=-4, xmin=-1.005, xmax=1.005, linewidth=1, color = 'k', linestyle = '-')
    plt.savefig(PATH + '/QR_B0_B0bar' + methodLabel + 'WithRes.pdf')
    fig3.clear()

    return 1


def plotWithResiduals(rooFitFrame, rooRealVar, dots, modelCurve, units, nameOfPlot, legend, removeArtifacts):

    # plot for Thesis
    rooFitFrame.Print()
    rooFitFrame.GetXaxis().SetTitle("")
    rooFitFrame.GetXaxis().SetLabelSize(0)

    rooFitFrame.GetYaxis().SetTitleSize(0.072)
    rooFitFrame.GetYaxis().SetTitleOffset(0.98)
    rooFitFrame.GetYaxis().SetLabelSize(0.055)

    pointsHist = ROOT.RooHist()

    iBin = 0

    # xValModel = ROOT.Double(-1.E30)
    # yValModel = ROOT.Double(-1.E30)
    xValDot = ROOT.Double(-1.E30)
    yValDot = ROOT.Double(-1.E30)

    iDotPoint = ROOT.RooRealVar("iDotPoint", "", 0.)
    iModelPoint = ROOT.RooRealVar("iModelPoint", "", 0.)
    iDotError = ROOT.RooRealVar("iDotError", "", 0.)
    # yComb = ROOT.RooFormulaVar("yComb", "", "-1*@0", ROOT.RooArgList(y1RV))
    iResidual = ROOT.RooFormulaVar("yComb", "", "(@0 - @1)/@2", ROOT.RooArgList(iDotPoint, iModelPoint, iDotError))

    while iBin < dots.GetN() - 1:
        dots.GetPoint(iBin, xValDot, yValDot)

        iDotPoint.setVal(yValDot)
        iModelPoint.setVal(modelCurve.interpolate(xValDot))
        iDotError.setVal(float(dots.GetErrorYlow(iBin) + dots.GetErrorYhigh(iBin)))

        if np.isnan(iResidual.getVal()) is not True:

            residualValue = iResidual.getVal()

            if removeArtifacts:
                if abs(residualValue) > 3:
                    residualValue = 0

            # print("xval = ", xValDot)
            # print(iBin, " = ", iResidual.getVal(), ", @0 = ", yValDot, ", @1 = ",
            #       modelCurve.interpolate(xValDot), ", @2 = ",
            #       float(dots.GetErrorYlow(iBin) + dots.GetErrorYhigh(iBin)))
            pointsHist.addBinWithXYError(xValDot, residualValue,
                                         dots.GetErrorXlow(iBin),
                                         dots.GetErrorXhigh(iBin),
                                         1, 1,)

        iBin += 1

    pointsHist.SetMarkerStyle(dots.GetMarkerStyle())
    pointsHist.SetMarkerSize(dots.GetMarkerSize())

    rooFitFrameRes = rooRealVar.frame()
    # rooFitFrameRes.addObject(mbcFrame.pullHist())
    # rooFitFrameRes.addPlotable(pointsHist, "P")

    rooFitFrameRes.SetTitle("")
    rooFitFrameRes.GetXaxis().SetTitle(rooRealVar.GetTitle() + " " + units)
    rooFitFrameRes.GetXaxis().SetTitleSize(0.2)
    rooFitFrameRes.GetXaxis().SetTitleOffset(0.9)

    rooFitFrameRes.GetXaxis().SetTickSize(0.07)
    rooFitFrameRes.GetYaxis().SetTickSize(0.024)

    rooFitFrameRes.GetYaxis().SetTitle("#splitline{Normalized}{ Residuals}")
    rooFitFrameRes.GetYaxis().SetTitleSize(0.16)
    rooFitFrameRes.GetYaxis().SetTitleOffset(0.4)

    rooFitFrameRes.GetXaxis().SetLabelSize(0.125)
    rooFitFrameRes.GetYaxis().SetLabelSize(0.120)

    rooFitFrameRes.SetAxisRange(-5, 5, "Y")
    rooFitFrameRes.GetYaxis().SetNdivisions(10)
    rooFitFrameRes.GetYaxis().ChangeLabel(1, -1, 0.)
    rooFitFrameRes.GetYaxis().ChangeLabel(3, -1, 0.)
    rooFitFrameRes.GetYaxis().ChangeLabel(5, -1, 0.)
    rooFitFrameRes.GetYaxis().ChangeLabel(7, -1, 0.)
    rooFitFrameRes.GetYaxis().ChangeLabel(9, -1, 0.)
    rooFitFrameRes.GetYaxis().ChangeLabel(11, -1, 0.)

    gLine1 = ROOT.TLine(5.2, 3, 5.295, 3)
    gLine2 = ROOT.TLine(5.2, 1, 5.295, 1)
    gLine3 = ROOT.TLine(5.2, -1, 5.295, -1)
    gLine4 = ROOT.TLine(5.2, -3, 5.295, -3)
    gLine1.SetLineColor(ROOT.kGreen + 3)
    gLine2.SetLineColor(ROOT.kBlue)
    gLine3.SetLineColor(ROOT.kBlue)
    gLine4.SetLineColor(ROOT.kGreen + 3)

    gLine1.SetLineWidth(2)
    gLine2.SetLineWidth(2)
    gLine3.SetLineWidth(2)
    gLine4.SetLineWidth(2)

    # print("Number of Y axis bins = ", rooFitFrameRes.GetYaxis().GetNbins() )

    c1 = ROOT.TCanvas("c1", "c1", 700, 640)
    c1.SetBottomMargin(0)
    c1.cd()

    Pad1 = ROOT.TPad("p1", "p1", 0, 0.277, 1, 1, 0)
    Pad2 = ROOT.TPad("p2", "p2", 0, 0, 1, 0.276, 0)
    Pad1.Draw()
    Pad2.Draw()

    Pad1.SetLeftMargin(0.15)
    Pad1.SetBottomMargin(0.02)
    Pad1.SetTopMargin(0.06)

    Pad2.SetLeftMargin(0.15)
    Pad2.SetBottomMargin(0.4)
    Pad2.SetTopMargin(0.01)

    Pad2.cd()
    rooFitFrameRes.Draw()
    gLine1.Draw("SAME")
    gLine2.Draw("SAME")
    gLine3.Draw("SAME")
    gLine4.Draw("SAME")
    pointsHist.Draw("P SAME")
    # Pad2.Update()

    Pad1.cd()

    rooFitFrame.Draw()

    if legend != "":
        legend.Draw()
    # Pad1.Update()
    nameOfPlot = nameOfPlot + "WithResiduals.pdf"
    c1.SaveAs(nameOfPlot)
    c1.Destructor()


for treename in treenames:

    print(treename)

    tdat = ROOT.TChain(treename)
    tMC = ROOT.TChain(treename)

    for iFile in workingFilesData:
        tdat.AddFile(iFile)

    for iFile in workingFilesMC:
        tMC.AddFile(iFile)

    histo_notTaggedEventsMC = ROOT.TH1F('notTaggedEventsMC',
                                        'Histogram for not tagged events',
                                        1, -3.0, -1.0)

    histo_notTaggedEventsData = ROOT.TH1F('notTaggedEventsData',
                                          'Histogram for not tagged events',
                                          1, -3.0, -1.0)

    tMC.Draw(
        'FBDT_qrCombined>>notTaggedEventsMC',
        'abs(qrMC) == 0 && isSignal == 1 && FBDT_qrCombined < -1')

    tdat.Draw(
        'FBDT_qrCombined>>notTaggedEventsData',
        'FBDT_qrCombined < -1')

    mc_total_notTagged = histo_notTaggedEventsMC.GetEntries()
    data_total_notTagged = histo_notTaggedEventsData.GetEntries()

    B0_mbc = ROOT.RooRealVar("Mbc", "#it{m}_{bc}", 0, 5.2, 5.295, "GeV/#it{c}^{2}")
    B0_deltaE = ROOT.RooRealVar("deltaE", "#Delta#it{E}", 0, -0.15, 0.15, "GeV/#it{c}^{2}")

    B0_FBDT_qrCombined = ROOT.RooRealVar("FBDT_qrCombined", "FBDT_qrCombined", 0, -1.01, 1.01)
    B0_FANN_qrCombined = ROOT.RooRealVar("FANN_qrCombined", "FANN_qrCombined", 0, -1.01, 1.01)

    B0_qrMC = ROOT.RooRealVar("qrMC", "qrMC", 0., -100, 100)

    argSet = ROOT.RooArgSet(B0_mbc, B0_deltaE)
    argSet.add(B0_FBDT_qrCombined)
    argSet.add(B0_FANN_qrCombined)
    argSet.add(B0_qrMC)

    cutData = "abs(FBDT_qrCombined) < 1.01"
    cutMC = "abs(qrMC) ==1 && abs(FBDT_qrCombined) < 1.01"

    data = ROOT.RooDataSet(
        "data",
        "data",
        tdat,
        argSet,
        cutData)

    mc = ROOT.RooDataSet(
        "mc",
        "mc",
        tMC,
        argSet,
        cutMC)

    fitMC = ROOT.RooDataSet("fitMC", "fitMC", ROOT.RooArgSet(B0_mbc, B0_FBDT_qrCombined, B0_FANN_qrCombined, B0_qrMC), "")
    fitData = ROOT.RooDataSet("fitData", "fitData", ROOT.RooArgSet(B0_mbc, B0_FBDT_qrCombined, B0_FANN_qrCombined), "")

    fitMC.append(mc)
    fitData.append(data)

    data_total_Tagged = fitData.numEntries()
    data_total_notTagged = histo_notTaggedEventsData.GetEntries()

    maxBo_mbc = tdat.GetMaximum("Mbc")  # maximum B0_mbc =  5.289213180541992
    print("maximum B0_mbc = ", maxBo_mbc)

    B0_mbc.setRange("fitRange", 5.2, maxBo_mbc)

#   Definition of Signal Components and Fit on MC

    mbcMuGauss1 = ROOT.RooRealVar("mbcMuGauss1", "mbcMuGauss1", 5.27943e+00, 5.27, 5.2893, "GeV")
    mbcSigmaGauss1 = ROOT.RooRealVar("mbcSigmaGauss1", "mbcSigmaGauss1", 2.62331e-03, 0, 0.1, "GeV")
    mbcGauss1 = ROOT.RooGaussModel("mbcGauss1", "mbcGauss1", B0_mbc, mbcMuGauss1, mbcSigmaGauss1)

    mbcMu2Shift = ROOT.RooRealVar("mbcMu2Shift", "mbcMu2Shift", 7.62270e-03, -0.01, 0.01)
    mbcSig2Factor = ROOT.RooRealVar("mbcSig2Factor", "mbcSig2Factor", 2.83547e-01, 0, 1)
    mbcMuGauss2 = ROOT.RooFormulaVar("mbcMuGauss2", "mbcMuGauss2", "@0-@1", ROOT.RooArgList(mbcMuGauss1, mbcMu2Shift))
    mbcSigmaGauss2 = ROOT.RooFormulaVar("mbcSigmaGauss2", "mbcSigmaGauss2", "@0/@1", ROOT.RooArgList(mbcSigmaGauss1, mbcSig2Factor))
    mbcGauss2 = ROOT.RooGaussModel("mbcGauss2", "mbcGauss2", B0_mbc, mbcMuGauss2, mbcSigmaGauss2)

    mbcFracGauss2 = ROOT.RooRealVar("mbcFracGauss2", "mbcFracGauss2", 9.93351e-01, 0.0, 1.)

    mbcSignalModel = ROOT.RooAddModel(
        "mbcSignalModel", "mbcSignalModel", ROOT.RooArgList(
            mbcGauss1, mbcGauss2), ROOT.RooArgList(mbcFracGauss2))

    mcFit = mbcSignalModel.fitTo(
        fitMC,
        ROOT.RooFit.Minos(ROOT.kFALSE), ROOT.RooFit.Extended(ROOT.kFALSE),
        ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    mbcMu2Shift.setConstant()
    mbcSig2Factor.setConstant()
    mbcFracGauss2.setConstant()

    mbcSignalArgset1 = ROOT.RooArgSet(mbcGauss1)
    mbcSignalArgset2 = ROOT.RooArgSet(mbcGauss2)

    mbcLogFrameMC = B0_mbc.frame()
    fitMC.plotOn(mbcLogFrameMC, ROOT.RooFit.LineWidth(4))
    mbcSignalModel.plotOn(mbcLogFrameMC)
    mbcSignalModel.plotOn(
        mbcLogFrameMC,
        ROOT.RooFit.Components(mbcSignalArgset1),
        ROOT.RooFit.LineColor(ROOT.kRed + 2), ROOT.RooFit.LineWidth(4))
    mbcSignalModel.plotOn(
        mbcLogFrameMC,
        ROOT.RooFit.Components(mbcSignalArgset2),
        ROOT.RooFit.LineColor(ROOT.kGreen + 3), ROOT.RooFit.LineWidth(4))

    mbcLogFrameMC.SetTitle("")
    mbcXtitle = B0_mbc.GetTitle() + " [GeV/#it{c}^{2}]"
    mbcLogFrameMC.GetXaxis().SetTitle(mbcXtitle)
    mbcLogFrameMC.GetXaxis().SetTitleSize(0.05)
    mbcLogFrameMC.GetXaxis().SetLabelSize(0.045)
    mbcLogFrameMC.GetYaxis().SetTitleSize(0.05)
    mbcLogFrameMC.GetYaxis().SetTitleOffset(1.5)
    mbcLogFrameMC.GetYaxis().SetLabelSize(0.045)

    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.SetLogy()
    Pad.Draw()
    Pad.cd()
    mbcLogFrameMC.Draw()
    Pad.Update()
    nPlot = PATH + "/B0_mbcMC.pdf"
    c1.SaveAs(nPlot)
    c1.Destructor()

#   Definition of Continuum component and fit of Signal Model and Continuum component on Data

    mbcSignYield = ROOT.RooRealVar("mbcSignYield", "mbcSignYield", 1.53208e+03, 0.0, 20000)
    mbcContYield = ROOT.RooRealVar("mbcContYield", "mbcContYield", 1.13456e+03, 0.0, 20000)

    mbcArgusMax = ROOT.RooRealVar("mbcArgusMax", "mbcArgusMax", maxBo_mbc, "GeV")
    mbcArgusC = ROOT.RooRealVar("mbcArgusC", "mbcArgusC", -4.65996e+01, -100, 0)
    mbcArgusPow = ROOT.RooRealVar("mbcArgusPow", "mbcArgusPow", 5.13442e-01, -2, 2, "GeV")
    mbcArgus = ROOT.RooArgusBG("mbcArgus", "mbcArgus", B0_mbc, mbcArgusMax, mbcArgusC, mbcArgusPow)

    mbcModel = ROOT.RooAddPdf(
        "mbcModel", "mbcModel", ROOT.RooArgList(
            mbcSignalModel, mbcArgus), ROOT.RooArgList(
            mbcSignYield, mbcContYield))

    dataFit = mbcModel.fitTo(
        fitData,
        ROOT.RooFit.Minos(ROOT.kFALSE), ROOT.RooFit.Extended(ROOT.kFALSE),
        ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    mbcMuGauss1.setConstant()
    mbcSigmaGauss1.setConstant()
    mbcArgusC.setConstant()
    mbcArgusPow.setConstant()

#   Now we use the SPlot class to add SWeights to our data set

    sData = ROOT.RooStats.SPlot("sData", "SPlot using B0_mbc", fitData, mbcModel, ROOT.RooArgList(mbcSignYield, mbcContYield))


#   Check that our weights have the desired properties

    print("Check SWeights:")

    print("Yield of mbcSignYield is ", mbcSignYield.getVal(), ".  From sWeights it is ",
          sData.GetYieldFromSWeight("mbcSignYield"))

    print("Yield of mbcContYield is ", mbcContYield.getVal(), ".  From sWeights it is ",
          sData.GetYieldFromSWeight("mbcContYield"))

    # for i in range(0, 10):
    #     print("mbcSignYield Weight  ", sData.GetSWeight(i,"mbcSignYield"),
    #               "   mbcContYield Weight   ", sData.GetSWeight(i,"mbcContYield"),
    #               "  Total Weight   ", sData.GetSumOfEventSWeight(i))

    dataFitExtended = mbcModel.fitTo(
        fitData, ROOT.RooFit.Extended(),
        ROOT.RooFit.Minos(ROOT.kFALSE), ROOT.RooFit.Extended(ROOT.kFALSE),
        ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    mbcModel.Print()

    mbcArgset1 = ROOT.RooArgSet(mbcArgus)
    mbcArgset2 = ROOT.RooArgSet(mbcSignalModel)

    B0_mbc.setBins(38)
    mbcFrame = B0_mbc.frame()
    fitData.plotOn(mbcFrame)
    mbcModel.plotOn(mbcFrame, ROOT.RooFit.LineWidth(4))
    mbcModel.plotOn(
        mbcFrame,
        ROOT.RooFit.Components(mbcArgset1),
        ROOT.RooFit.LineColor(
            ROOT.kRed + 2),
        ROOT.RooFit.LineStyle(5),
        ROOT.RooFit.LineWidth(4))
    mbcModel.plotOn(
        mbcFrame,
        ROOT.RooFit.Components(mbcArgset2),
        ROOT.RooFit.LineColor(
            ROOT.kGreen + 3),
        ROOT.RooFit.LineStyle(7),
        ROOT.RooFit.LineWidth(4))

    mbcFrame.SetTitle("")
    mbcFrame.GetXaxis().SetTitle(mbcXtitle)
    mbcFrame.SetTitleOffset(0.9, "X")
    mbcFrame.GetXaxis().SetTitleSize(0.075)
    mbcFrame.GetXaxis().SetLabelSize(0.045)
    mbcFrame.GetYaxis().SetTitleSize(0.06)
    mbcFrame.GetYaxis().SetTitleOffset(0.95)
    mbcFrame.GetYaxis().SetLabelSize(0.045)

    c1 = ROOT.TCanvas("c1", "c1", 10, 15, 700, 500)
    c1.SetLeftMargin(0.15)
    c1.SetRightMargin(0.04)
    c1.SetTopMargin(0.03)
    c1.SetBottomMargin(0.15)
    c1.cd()
    # Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    # Pad.SetLeftMargin(0.15)
    # Pad.SetBottomMargin(0.15)
    # Pad.Draw()
    # Pad.cd()
    mbcFrame.Draw()
    # Pad.Update()
    nPlot = PATH + "/B0_mbc.pdf"
    c1.SaveAs(nPlot)
    c1.Destructor()

    # Plot Mbc MC with residuals
    mbcFrameMC = B0_mbc.frame()
    mbcFrameMC.SetTitle("")

    fitMC.plotOn(mbcFrameMC)
    mbcSignalModel.plotOn(mbcFrameMC, ROOT.RooFit.LineColor(ROOT.kGreen + 3), ROOT.RooFit.LineStyle(7), ROOT.RooFit.LineWidth(4))

    dotsMC = mbcFrameMC.getHist("h_fitMC")
    modelCurveMC = mbcFrameMC.getCurve("mbcSignalModel_Norm[Mbc]")

    dotsMC.SetFillColor(ROOT.kWhite)
    modelCurveMC.SetFillColor(ROOT.kWhite)

    mbcFrameMC.Print()

    lMC = ROOT.TLegend(0.25, 0.63, 0.5, 0.89)
    lMC.AddEntry(dotsMC, "Belle MC")
    lMC.AddEntry(modelCurveMC, 'Signal')

    lMC.SetTextSize(0.055)

    plotWithResiduals(mbcFrameMC, B0_mbc, dotsMC, modelCurveMC, "[GeV/#it{c}^{2}]", "mbcMC", lMC, True)

    # Plot of Model with Mbc and residuals

    dots = mbcFrame.getHist("h_fitData")
    modelCurve = mbcFrame.getCurve("mbcModel_Norm[Mbc]")
    signalCurve = mbcFrame.getCurve("mbcModel_Norm[Mbc]_Comp[mbcSignalModel]")
    continuumCurve = mbcFrame.getCurve("mbcModel_Norm[Mbc]_Comp[mbcArgus]")

    dots.SetFillColor(ROOT.kWhite)
    modelCurve.SetFillColor(ROOT.kWhite)
    signalCurve.SetFillColor(ROOT.kWhite)
    continuumCurve.SetFillColor(ROOT.kWhite)

    legend = ROOT.TLegend(0.25, 0.43, 0.5, 0.89)
    legend.AddEntry(dots, "Belle data")
    legend.AddEntry(modelCurve, 'Fit result')
    legend.AddEntry(signalCurve, "Signal")
    legend.AddEntry(continuumCurve, 'Continuum')

    legend.SetTextSize(0.055)

    plotWithResiduals(mbcFrame, B0_mbc, dots, modelCurve, "[GeV/#it{c}^{2}]", "Mbc", legend, False)

    signalData = ROOT.RooDataSet("signalData", "signalData", fitData, fitData.get(), "", "mbcSignYield_sw")

    print("Variable Names")

    qrSigFrame = B0_FBDT_qrCombined.frame()
    signalData.plotOn(qrSigFrame, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    qrSigFrame.SetTitle("")
    mbcXtitle = "qr"
    qrSigFrame.GetXaxis().SetTitle(mbcXtitle)
    qrSigFrame.GetXaxis().SetTitleSize(0.05)
    qrSigFrame.GetXaxis().SetLabelSize(0.045)
    qrSigFrame.GetYaxis().SetTitleSize(0.05)
    qrSigFrame.GetYaxis().SetTitleOffset(1.5)
    qrSigFrame.GetYaxis().SetLabelSize(0.045)

    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    qrSigFrame.Draw()

    Pad.Update()
    nPlot = PATH + "/B0_qrSignal.pdf"
    c1.SaveAs(nPlot)
    c1.Destructor()

    continuumData = ROOT.RooDataSet("continuumData", "continuumData", fitData, fitData.get(), "", "mbcContYield_sw")

    qrContFrame = B0_FBDT_qrCombined.frame()
    continuumData.plotOn(qrContFrame, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    qrContFrame.SetTitle("")
    mbcXtitle = "qr"
    qrContFrame.GetXaxis().SetTitle(mbcXtitle)
    qrContFrame.GetXaxis().SetTitleSize(0.05)
    qrContFrame.GetXaxis().SetLabelSize(0.045)
    qrContFrame.GetYaxis().SetTitleSize(0.05)
    qrContFrame.GetYaxis().SetTitleOffset(1.5)
    qrContFrame.GetYaxis().SetLabelSize(0.045)

    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    qrContFrame.Draw()
    Pad.Update()
    nPlot = PATH + "/B0_qrContinuum.pdf"
    c1.SaveAs(nPlot)
    c1.Destructor()

    fitData.Print()

    fitMC.Print()

    sPlotResultsFBDT = np.zeros((fitData.numEntries(), 3))
    sPlotResultsFANN = np.zeros((fitData.numEntries(), 3))
    for i in range(fitData.numEntries()):
        row = fitData.get(i)
        sPlotResultsFBDT[i][0] = row.getRealValue('FBDT_qrCombined', 0, ROOT.kTRUE)
        sPlotResultsFBDT[i][1] = row.getRealValue('mbcSignYield_sw', 0, ROOT.kTRUE)
        sPlotResultsFBDT[i][2] = row.getRealValue('mbcContYield_sw', 0, ROOT.kTRUE)
        sPlotResultsFANN[i][0] = row.getRealValue('FANN_qrCombined', 0, ROOT.kTRUE)
        sPlotResultsFANN[i][1] = row.getRealValue('mbcSignYield_sw', 0, ROOT.kTRUE)
        sPlotResultsFANN[i][2] = row.getRealValue('mbcContYield_sw', 0, ROOT.kTRUE)

    mcSPlotFBDT = np.zeros((fitMC.numEntries(), 2))
    mcSPlotFANN = np.zeros((fitMC.numEntries(), 2))
    for i in range(fitMC.numEntries()):
        row = fitMC.get(i)
        mcSPlotFBDT[i][0] = row.getRealValue('FBDT_qrCombined', 0, ROOT.kTRUE)
        mcSPlotFBDT[i][1] = row.getRealValue('qrMC', 0, ROOT.kTRUE)
        mcSPlotFANN[i][0] = row.getRealValue('FANN_qrCombined', 0, ROOT.kTRUE)
        mcSPlotFANN[i][1] = row.getRealValue('qrMC', 0, ROOT.kTRUE)

    efficiencyCalculator(mcSPlotFBDT, sPlotResultsFBDT, "FBDT")
    efficiencyCalculator(mcSPlotFANN, sPlotResultsFANN, "FANN")
