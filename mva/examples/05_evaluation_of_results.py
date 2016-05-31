#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import root_pandas
from B2Tools import b2plot
import sklearn.metrics

if __name__ == "__main__":

    data = root_pandas.read_root('ntuple.root')

    p = b2plot.PurityOverEfficiency()
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
    p.xmin = 0.8
    p.ymin = 0.5
    p.finish()
    p.save('evaluation.png')

    print('AUC (Ordinary)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boOrdinary__bc))
    print('AUC (Full)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boFull__bc))
    print('AUC (Pdf)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boPdf__bc))
    print('AUC (SPlot)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boSPlot__bc))
    print('AUC (SPlotCombined)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boSPlotCombined__bc))
    print('AUC (SPlotBoosted)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boSPlotBoosted__bc))
    print('AUC (SPlotCombinedBoosted)', sklearn.metrics.roc_auc_score(data.isSignal, data.extraInfo__boSPlotCombinedBoosted__bc))
