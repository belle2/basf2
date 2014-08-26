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
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle for every channel
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    ROOT.gROOT.SetBatch(True)
    files = [ROOT.TFile(filename, 'UPDATE') for filename, _ in preCutHistograms]
    signal, bckgrd, ratio = LoadHistogramsFromFiles(files, preCutConfig.variable, channelNames, preCutHistograms)
    interpolations = GetInterpolateFunctions(ratio)

    # Two-Side cut S/B ratio constructed
    if preCutConfig.method == 'S/B' and preCutConfig.variable in ['M', 'Q', 'Mbc']:
        maxima = GetPositionsOfMaxima(signal)

        def ycut_to_xcuts(channel, cut):
            return [interpolations[channel].GetX(cut, signal[channel].GetXaxis().GetXmin(), maxima[channel]),
                    interpolations[channel].GetX(cut, maxima[channel], signal[channel].GetXaxis().GetXmax())]

    # One-Side cut S/B ratio constructed
    elif preCutConfig.method == 'S/B' and preCutConfig.variable in ['daughterProductOf(getExtraInfo(SignalProbability))']:

        def ycut_to_xcuts(channel, cut):
            return [interpolations[channel].GetX(cut, 0, 1), 1]

    # Two sided cut, same for all channels
    elif preCutConfig.method == 'Same' and preCutConfig.variable in ['M', 'Q', 'Mbc']:
        maxima = GetPositionsOfMaxima(signal)

        def ycut_to_xcuts(channel, cut):
            return [maxima[channel] - cut * 2, maxima[channel] + cut * 2]

    # One sided cut, same for all channels
    elif preCutConfig.method == 'Same' and preCutConfig.variable in ['daughterProductOf(getExtraInfo(SignalProbability))']:

        def ycut_to_xcuts(channel, cut):
            return [cut, 1]

    else:
        raise RuntimeError('Given PreCutConfiguration is not implemented. Please check that method and used variables are compatible.')

    result = {}
    redo_cuts = True
    while redo_cuts:
        redo_cuts = False

        cuts = GetCuts(signal, bckgrd, preCutConfig.efficiency, preCutConfig.purity, ycut_to_xcuts)
        for (channel, range) in cuts.iteritems():
            # Modify the range to match the bin boundaries
            axis = signal[channel].GetXaxis()
            range[0] = axis.GetBinLowEdge(axis.FindBin(range[0]))
            range[1] = axis.GetBinUpEdge(axis.FindBin(range[1]))
            result[channel] = {'range': range, 'isIgnored': False,
                               'cutstring': str(range[0]) + " <= " + preCutConfig.variable + " <= " + str(range[1]),
                               'nBackground': GetNumberOfEventsInRange(bckgrd[channel], range),
                               'nSignal': GetNumberOfEventsInRange(signal[channel], range)}

        for ignoredChannel in GetIgnoredChannels(signal, bckgrd, cuts):
            redo_cuts = True
            result[ignoredChannel]['isIgnored'] = True
            B2WARNING("Ignoring channel " + ignoredChannel + "!")

        signal = {channel: hist for (channel, hist) in signal.iteritems() if not result[channel]['isIgnored']}
        bckgrd = {channel: hist for (channel, hist) in bckgrd.iteritems() if not result[channel]['isIgnored']}

    return result


def LoadHistogramsFromFiles(files, variable, channelNames, preCutHistograms):
    """
    Load for every channel the signal and background histogram of a variable from given ROOT file and
    calculates the ratio S/B histogram.
    @param filename filename of the ROOT file containing the histograms
    @param variable the variable which defines the x-axis of the histograms
    @param channels the channels for which the histograms are loaded
    """
    signal = dict([(channel, file.GetKey('signal' + key).ReadObj()) for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])
    all = dict([(channel, file.GetKey('all' + key).ReadObj()) for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])

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


def GetCuts(signal, bckgrd, efficiency, purity, ycut_to_xcuts):
    """
    Calculates the individual cuts on the x-axis for all channels
    @param signal signal histograms of the channels
    @param bckgrd signal histograms of the channels
    @param efficiency the maximal efficiency for the channels in total
    @param purity the minimal purity for the channels in total
    @param ycut_to_xcuts function which calculates xcuts from a given cut on the y-axis
    """

    nSignal = sum([value.GetEntries() for value in signal.values()])

    if nSignal == 0:
        channels = [channel for channel in signal.iterkeys()]
        B2WARNING("No signal present in any of these channels: " + (", ".join(channels)))
        return {channel: (0, 0) for channel in signal.iterkeys()}

    def pythonEfficiencyFunc(ycut):
        return sum([GetNumberOfEventsInRange(s, ycut_to_xcuts(channel, ycut[0])) for (channel, s) in signal.iteritems()]) / float(nSignal)

    def pythonPurityFunc(ycut):
        s = sum([GetNumberOfEventsInRange(h, ycut_to_xcuts(channel, ycut[0])) for (channel, h) in signal.iteritems()])
        b = sum([GetNumberOfEventsInRange(h, ycut_to_xcuts(channel, ycut[0])) for (channel, h) in bckgrd.iteritems()])
        if s == 0:
            return 0
        if b == 0:
            return 1
        return s / float(s + b)

    rootEfficiencyFunc = ROOT.TF1('EfficiencyFunction', pythonEfficiencyFunc, 0, 1, 0)
    ycut = rootEfficiencyFunc.GetX(efficiency, 0.0, 1.0)

    if pythonPurityFunc([ycut]) < purity:
        rootPurityFunc = ROOT.TF1('PurityFunction', pythonPurityFunc, 0, 1, 0)
        ycut = rootPurityFunc.GetX(purity, 0.0, 1.0)

    return {channel: ycut_to_xcuts(channel, ycut) for channel in signal.iterkeys()}


def GetNumberOfEventsInRange(histogram, (a, b)):
    """
    Calculates the number of events in the given histograms between a and b
    Beware of nasty bug here, if b is exactly the upper edge of a bin, we had to make sure
    that we don't count one bin more than we should!
    @param histograms Histograms
    @param a lower boundary
    @param b upper boundary
    """
    low = histogram.FindBin(a)
    if histogram.FindBin(a + histogram.GetBinWidth(low) / 2.0) != low:
        low += 1
    high = histogram.FindBin(b)
    if histogram.FindBin(b - histogram.GetBinWidth(high) / 2.0) != high:
        high -= 1
    return histogram.Integral(low, high)


def GetNumberOfEvents(histogram):
    """
    Calculates the number of events in the given histograms includeing underflow and overflow bin
    @param histograms Histograms
    """
    return histogram.Integral(0, histogram.GetNbinsX() + 1)


def GetIgnoredChannels(signal, bckgrd, cuts):
    """
    Returns the channels which should be ignored because the cuts are very tight and therefore
    there's not enough statistic for a training.
    @param signal signal histograms of the channels
    @param bckgrd background histograms of the channels
    @param cuts cuts on the x-axis of the channels
    """
    def isIgnored(channel):
        return abs(cuts[channel][0] - cuts[channel][1]) < 1e-4 or GetNumberOfEventsInRange(signal[channel], cuts[channel]) < 100 or GetNumberOfEventsInRange(bckgrd[channel], cuts[channel]) < 100
    return [channel for channel in signal.iterkeys() if isIgnored(channel)]
