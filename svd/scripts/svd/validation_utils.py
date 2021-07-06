#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import fnmatch
import math

import numpy as np
import matplotlib
import matplotlib.pyplot as plt

import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True

matplotlib.use('Agg')
plt.style.use("belle2")


def get_full_path(name, exp, run, base_dir):
    return f'{base_dir}/{name}/{name}_{exp}.{run}/{name}_1'


def get_combined(histos, pattern):
    '''
    Sum histograms that match a certain pattern shell-style, e.g.
    "*" all, "L3*" all layer 3, "*U" all U-sensors

    Parameters:
    histos (dict{label : TH1}): dictionary of
    patern (str): shell-styel matching pattern
    '''

    good_keys = [key for key in histos.keys() if fnmatch.fnmatch(key, pattern)]

    assert len(good_keys) > 0

    h = histos[good_keys[0]].Clone()
    h.SetName(h.GetName().split('__')[0]+f'__{pattern}')
    h.SetTitle(h.GetName().split('__')[0]+f' in {pattern}')

    for key in good_keys[1:]:
        h.Add(histos[key])

    return h


def get_histo_offTracks(histo_all, histo_onTracks):
    '''
    Substract the histogram with the clusters on tracks from the histogram with all the clusters
    to get the histogram with the clusters off tracks
    '''
    try:
        histo_offTracks = histo_all.Clone(histo_all.GetName().replace('all', 'offTracks'))
        histo_offTracks.SetTitle(histo_offTracks.GetTitle().replace('All', 'OffTracks'))
        histo_offTracks.Add(histo_onTracks, -1)
        return histo_offTracks
    except (ReferenceError, TypeError):
        return None


def get_agreament(histo_eventT0, histo_diff, min_entries=100):
    '''
    Get the mean of the difference between the mean time of the clusters on tracks and
    the mean eventT0 divided by the RMS of the event T0
    '''
    if histo_eventT0.GetEntries() > min_entries and histo_diff.GetEntries() > min_entries:
        return histo_diff.GetMean()/histo_eventT0.GetRMS()
    else:
        return np.nan


def get_precision(histo_diff, min_entries=100):
    '''
    Get the RMS of the difference between the mean time of the clusters on tracks
    '''
    if histo_diff.GetEntries() > min_entries:
        return histo_diff.GetRMS()
    else:
        return np.nan


def get_agreament2(histo_eventT0, histo_onTracks, min_entries=100):
    '''
    Get the difference between the mean time of the clusters on tracks and the mean eventT0 divided by the RMS of the event T0
    '''
    # Get the difference in number of sigmas between the mean eventT0 and the mean time of the clusters on tracks
    if histo_eventT0.GetEntries() > min_entries and histo_onTracks.GetEntries() > min_entries:
        return (histo_onTracks.GetMean() - histo_eventT0.GetMean())/histo_eventT0.GetRMS()
    else:
        return np.nan


def make_roc(hist_sgn, hist_bkg, lower_is_better=False, two_sided=True):
    import root_numpy
    dist_sgn = root_numpy.hist2array(hist_sgn)
    dist_bkg = root_numpy.hist2array(hist_bkg)
    dist_sgn = dist_sgn/dist_sgn.sum()
    dist_bkg = dist_bkg/dist_bkg.sum()
    if two_sided:
        dist_sgn = np.append(dist_sgn, [0])
        dist_bkg = np.append(dist_bkg, [0])
        eff_sgn = tuple(reversed([sum(dist_sgn[i:-(1+i)]) for i in range(math.ceil(len(dist_sgn)/2)+1)]))
        eff_bkg = tuple(reversed([sum(dist_bkg[i:-(1+i)]) for i in range(math.ceil(len(dist_bkg)/2)+1)]))
    else:
        if not lower_is_better:
            dist_sgn = np.array(tuple(reversed(dist_sgn)))
            dist_bkg = np.array(tuple(reversed(dist_bkg)))
        eff_sgn = [sum(dist_sgn[:i+1]) for i in range(len(dist_sgn))]
        eff_bkg = [sum(dist_bkg[:i+1]) for i in range(len(dist_bkg))]
    return eff_sgn, [1-i for i in eff_bkg]


def get_roc_auc(hist_sgn, hist_bkg, lower_is_better=False, two_sided=True, min_entries=100):
    if hist_sgn.GetEntries() > min_entries and hist_bkg.GetEntries() > min_entries:

        return np.trapz(*reversed(make_roc(
            hist_sgn, hist_bkg,
            lower_is_better=lower_is_better,
            two_sided=two_sided)))
    else:
        return np.nan


def np2plt_hist(np_hist):
    return {'x': np_hist[1][0][:-1], 'bins': np_hist[1][0], 'weights': np_hist[0]}


def make_combined_plot(pattern, histos, title=None):
    import root_numpy
    h_onTracks = root_numpy.hist2array(get_combined(histos['onTracks'], pattern), return_edges=True)
    h_offTracks = root_numpy.hist2array(get_combined(histos['offTracks'], pattern), return_edges=True)
    h_eventT0 = root_numpy.hist2array(histos['eventT0'], return_edges=True)

    # normalise h_eventT0 to have same number of entries as h_onTracks
    h_eventT0 = (h_eventT0[0]*sum(h_onTracks[0])/sum(h_eventT0[0]), h_eventT0[1])

    plt.figure()
    plt.hist(h_onTracks[1][0][:-1], h_onTracks[1][0], weights=h_onTracks[0], histtype='step', label='on tracks')
    plt.hist(h_offTracks[1][0][:-1], h_offTracks[1][0], weights=h_offTracks[0], histtype='step', label='off tracks')
    plt.hist(h_eventT0[1][0][:-1], h_eventT0[1][0], weights=h_eventT0[0], histtype='step', label='event T0')
    plt.legend(loc='upper right')
    plt.xlabel('cluster time [ns]')
    if title is not None:
        plt.title(title)
    plt.tight_layout()


# get labels for all possible sides
names_sides = []
ladder_of_layer = [7, 10, 12, 16]
sensor_on_layer = [2, 3, 4, 5]
for layer in range(3, 7):
    for ladder in range(1, ladder_of_layer[layer-3]+1):
        for sensor in range(1, sensor_on_layer[layer-3]+1):
            for side in ['U', 'V']:
                names_sides.append(f'L{layer}L{ladder}S{sensor}{side}')


# grouped sensors integrating phi
names_grouped_sides = []
for layer in range(3, 7):
    for sensor in range(1, sensor_on_layer[layer-3]+1):
        for side in ['U', 'V']:
            names_grouped_sides.append(f'L{layer}L*S{sensor}{side}')


time_algorithms = ['CoG6', 'CoG3', 'ELS3']


def get_histos(in_file, algo='CoG3',
               names_sides=names_sides,
               base_dirs={algo: f'SVDTimeValidationCollector_{algo}'
                          for algo in time_algorithms}):

    histos = {}

    base_dir = base_dirs[algo]
    iov = in_file.Get(f'{base_dir}/RunRange').getIntervalOfValidity()
    exp, run = iov.getExperimentLow(), iov.getRunLow()

    histos['eventT0'] = in_file.Get(get_full_path('hEventT0', exp, run, base_dir))

    histos_all = {name_side: in_file.Get(get_full_path(f'clsTimeAll__{name_side}', exp, run, base_dir))
                  for name_side in names_sides}
    histos['onTracks'] = {name_side: in_file.Get(get_full_path(f'clsTimeOnTracks__{name_side}', exp, run, base_dir))
                          for name_side in names_sides}
    histos['offTracks'] = {key: get_histo_offTracks(histos_all[key], histos['onTracks'][key])
                           for key in histos['onTracks']}
    histos['diff'] = {name_side: in_file.Get(get_full_path(f'clsDiffTimeOnTracks__{name_side}', exp, run, base_dir))
                      for name_side in names_sides}

    # replace None with empty histograms
    for kind in ['onTracks', 'offTracks', 'diff']:
        for key, value in histos[kind].items():
            if not isinstance(value, r.TH1):
                histos[kind][key] = r.TH1F(f'{kind}_{key}_{algo}', f'{kind}_{key}_{algo}', 300, -150, 150)

    return histos, exp, run
