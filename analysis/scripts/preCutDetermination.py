#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

import ROOT
import os
import hashlib
from basf2 import *


def CalculatePreCuts(preCutConfig, channelNames, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle. The cuts are chosen as follows:
        1. The ratio of signal/background child probability product preCut_Histogram is calculated and fitted with splines
        2. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is garantueed.
        3. The cut on the y-axis is converted to a minimum cut on the x-axis for every channel.
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    ROOT.gROOT.SetBatch(True)
    files = [ROOT.TFile(filename, 'UPDATE') for filename, _ in preCutHistograms]

    if preCutConfig.variable == 'Mass':
        variable = 'M'
    else:
        variable = 'daughterProductOf(getExtraInfo(SignalProbability))'

    signal, bckgrd, ratio = LoadHistogramsFromFiles(files, variable, channelNames, preCutHistograms)
    interpolations = GetInterpolateFunctions(ratio)

    if preCutConfig.variable == 'Mass':
        maxima = GetPositionsOfMaxima(signal)

        def ycut_to_xcuts(channel, cut):
            return (interpolations[channel].GetX(cut, signal[channel].GetXaxis().GetXmin(), maxima[channel]), interpolations[channel].GetX(cut, maxima[channel], signal[channel].GetXaxis().GetXmax()))
    else:

        def ycut_to_xcuts(channel, cut):
            return (interpolations[channel].GetX(cut, 0, 1), 1)

    cuts = GetCuts(signal, preCutConfig.efficiency, ycut_to_xcuts)

    result = {channel: {'variable': variable, 'range': range, 'isIgnored': False,
                        'nBackground': GetNumberOfEventsInRange(bckgrd[channel], range),
                        'nSignal': GetNumberOfEventsInRange(signal[channel], range)} for (channel, range) in cuts.iteritems()}
    for ignoredChannel in GetIgnoredChannels(signal, bckgrd, cuts):
        result[ignoredChannel]['isIgnored'] = True
        B2WARNING("Ignoring channel " + ignoredChannel + "!")
    return result


def LoadHistogramsFromFiles(files, variable, channelNames, preCutHistograms):
    """
    Load for every channel the signal and background histogram of a variable from given ROOT file and
    calculates the ratio S/B histogram.
    @param filename filename of the ROOT file containing the histograms
    @param variable the variable which defines the x-axis of the histograms
    @param channels the channels for which the histograms are loaded
    """
    signal = dict([(channel, file.Get('signal' + key)) for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])
    all = dict([(channel, file.Get('all' + key)) for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])

    if any([hist is None for hist in signal.values() + all.values()]):
        raise RuntimeError('Error while opening ROOT file with preCut Histograms')

    bckgrd = dict([(channel, value.Clone('bckgrd' + channel)) for (channel, value) in all.iteritems()])
    ratio = dict([(channel, value.Clone('ratio' + channel)) for (channel, value) in signal.iteritems()])

    for channel, file in zip(channelNames, files):
        file.cd()
        bckgrd[channel].Add(signal[channel], -1)
        bckgrd[channel].SetTitle('Background')
        bckgrd[channel].Write('', ROOT.TObject.kOverwrite)
        ratio[channel].Divide(bckgrd[channel])
        ratio[channel].SetTitle('Ratio')
        ratio[channel].Write('', ROOT.TObject.kOverwrite)
    return (signal, bckgrd, ratio)


def GetPositionsOfMaxima(histograms):
    """
    Calculates the Positions of the Maxima of the given histograms
    @param histograms Histograms
    """
    return dict([(channel, value.GetBinCenter(value.GetMaximumBin())) for (channel, value) in histograms.iteritems()])


def GetInterpolateFunctions(histograms):
    """
    GetInterpolate Functions to the given histograms
    @param histograms Histograms
    """
    return dict([(
                channel,
                ROOT.TF1(
                    hashlib.sha1(channel).hexdigest() + '_func',
                    lambda x, value=value: value.Interpolate(x[0]),
                    value.GetXaxis().GetXmin(),
                    value.GetXaxis().GetXmax(),
                    0
                )
                ) for (channel, value) in histograms.iteritems()])


def GetFitFunctions(histograms):
    """
    GetFit Functions to the given histograms
    @param histograms Histograms
    """
    dict = {}
    for (channel, value) in histograms.iteritems():
        fitfunc = ROOT.TF1(hashlib.sha1(channel).hexdigest() + '_func',
                           'gaus(0)',
                           value.GetXaxis().GetXmin(),
                           value.GetXaxis().GetXmax())
        fitfunc.SetParameter(0, value.Integral() / 6)
        fitfunc.SetParameter(1, value.GetBinCenter(value.GetMaximumBin()))
        fitfunc.SetParameter(2, 1)
        value.Fit(fitfunc)
        dict[channel] = fitfunc
    return dict


def GetCuts(signal, efficiency, ycut_to_xcuts):
    """
    Calculates the individual cuts on the x-axis for all channels
    @param signal signal histograms of the channels
    @param efficiency the minimal efficiency for each channel
    @param ycut_to_xcuts function which calculates xcuts from a given cut on the y-axis
    """
    nDesiredSignal = sum([value.GetEntries() for value in signal.values()]) * efficiency

    def nSignal_after_cut(ycut):
        return sum([GetNumberOfEventsInRange(s, ycut_to_xcuts(channel, ycut[0])) for (channel, s) in signal.iteritems()])

    f = ROOT.TF1('Cut_function', nSignal_after_cut, 0, 1, 0)
    ycut = f.GetX(nDesiredSignal, 0.0, 1.0)
    return {channel: ycut_to_xcuts(channel, ycut) for channel in signal.iterkeys()}


def GetNumberOfEventsInRange(histogram, (a, b)):
    """
    Calculates the number of events in the given histograms between a and b
    @param histograms Histograms
    @param a lower boundary
    @param b upper boundary
    """
    return histogram.Integral(histogram.FindBin(a), histogram.FindBin(b))


def GetIgnoredChannels(signal, bckgrd, cuts):
    """
    Returns the channels which should be ignored because the cuts are very tight and therefore
    there's not enough statistic for an training.
    @param signal signal histograms of the channels
    @param bckgrd background histograms of the channels
    @param cuts cuts on the x-axis of the channels
    """
    def isIgnored(channel):
        return cuts[channel][0] == cuts[channel][1] or GetNumberOfEventsInRange(signal[channel], cuts[channel]) < 100 or GetNumberOfEventsInRange(bckgrd[channel], cuts[channel]) < 100
    return [channel for channel in signal.iterkeys() if isIgnored(channel)]
