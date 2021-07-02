#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# You can use the basf2_mva_evaluation.py tool as well

from basf2_mva_util import tree2dict, calculate_roc_auc
from basf2_mva_evaluation import plotting
import ROOT

if __name__ == "__main__":
    rootchain = ROOT.TChain("variables")
    rootchain.Add('ntuple.root')

    variables = ['extraInfo__boFull__bc', 'extraInfo__boOrdinary__bc', 'extraInfo__boPdf__bc', 'extraInfo__boSPlot__bc',
                 'extraInfo__boSPlotCombined__bc', 'extraInfo__boSPlotBoosted__bc', 'extraInfo__boSPlotCombinedBoosted__bc',
                 'isSignal']
    data = tree2dict(rootchain, variables, variables)

    p = plotting.RejectionOverEfficiency()
    p.set_plot_options(dict(linestyle='-', lw=6))
    p.set_errorband_options(None)
    p.set_errorbar_options(None)
    p.add(data, 'extraInfo__boFull__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boOrdinary__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boPdf__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boSPlot__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boSPlotCombined__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boSPlotBoosted__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'extraInfo__boSPlotCombinedBoosted__bc', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('evaluation.png')

    print('AUC (Ordinary)', calculate_roc_auc(data['extraInfo__boOrdinary__bc'], data['isSignal']))
    print('AUC (Full)', calculate_roc_auc(data['extraInfo__boFull__bc'], data['isSignal']))
    print('AUC (Pdf)', calculate_roc_auc(data['extraInfo__boPdf__bc'], data['isSignal']))
    print('AUC (SPlot)', calculate_roc_auc(data['extraInfo__boSPlot__bc'], data['isSignal']))
    print('AUC (SPlotCombined)', calculate_roc_auc(data['extraInfo__boSPlotCombined__bc'], data['isSignal']))
    print('AUC (SPlotBoosted)', calculate_roc_auc(data['extraInfo__boSPlotBoosted__bc'], data['isSignal']))
    print('AUC (SPlotCombinedBoosted)', calculate_roc_auc(data['extraInfo__boSPlotCombinedBoosted__bc'], data['isSignal']))
