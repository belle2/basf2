#!/usr/bin/env python3
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
        1. The ratio signal/background over the cut variable is calculated from histograms
        2. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is guaranteed.
        3. If the purity requirement in the config is not met, the y-axis cut is determined from purity instead.
        4. The cut on the y-axis is converted to a minimum cut on the x-axis for every channel.
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle for every channel
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    ROOT.gROOT.SetBatch(True)
    files = [ROOT.TFile(filename, 'UPDATE') for filename, _ in preCutHistograms]
    signal, bckgrd, ratio = LoadHistogramsFromFiles(files, preCutConfig.variable, channelNames, preCutHistograms)

    # Two-Side cut S/B ratio constructed
    def ycut_to_xcuts(channel, cut):
        hist = ratio[channel]
        maximum = hist.GetMaximumBin()
        if hist.GetBinContent(maximum) < cut:
            return [0, -1]
        minimum = hist.GetMinimumBin()
        axis = hist.GetXaxis()
        if hist.GetBinContent(minimum) > cut:
            return [axis.GetBinLowEdge(1), axis.GetBinUpEdge(hist.GetNbinsX())]
        low = [bin for bin in range(1, maximum + 1) if hist.GetBinContent(bin) < cut]
        high = [bin for bin in range(maximum, hist.GetNbinsX() + 1) if hist.GetBinContent(bin) < cut]
        return [axis.GetBinLowEdge(max(low) + 1 if low else 1), axis.GetBinUpEdge(min(high) - 1 if high else hist.GetNbinsX())]

    print("Total number of signals", sum([value.GetEntries() for value in list(signal.values())]))
    print("Total number of background", sum([value.GetEntries() for value in list(bckgrd.values())]))

    result = {}
    redo_cuts = True
    while redo_cuts:
        redo_cuts = False

        cuts = GetCuts(signal, bckgrd, preCutConfig.efficiency, preCutConfig.purity, ycut_to_xcuts)
        for (channel, channel_range) in cuts.items():
            result[channel] = {
                'range': channel_range, 'isIgnored': False,
                'cutstring': str(channel_range[0]) + " <= " + preCutConfig.variable + " <= " + str(channel_range[1]),
                'nBackground': GetNumberOfEventsInRange(bckgrd[channel], channel_range),
                'nSignal': GetNumberOfEventsInRange(signal[channel], channel_range),
                'signalPeak': signal[channel].GetMean(),
                'signalWidth': signal[channel].GetRMS(),
                'variable': preCutConfig.variable
            }

        for ignoredChannel in GetIgnoredChannels(signal, bckgrd, cuts):
            redo_cuts = True
            result[ignoredChannel]['isIgnored'] = True
            B2WARNING("Ignoring channel " + ignoredChannel + "!")

        signal = {channel: hist for (channel, hist) in signal.items() if not result[channel]['isIgnored']}
        bckgrd = {channel: hist for (channel, hist) in bckgrd.items() if not result[channel]['isIgnored']}

    signalsum = 0
    backgroundsum = 0
    for k, v in result.items():
        signalsum += v['nSignal']
        backgroundsum += v['nBackground']
        print(k, ': ', v['cutstring'])
    print("Total number of signals after cut", signalsum)
    print("Total number of background after cut", backgroundsum)

    return result


def LoadHistogramsFromFiles(files, variable, channelNames, preCutHistograms):
    """
    Load for every channel the signal and background histogram of a variable from given ROOT file and
    calculates the ratio S/B histogram.
    @param filename filename of the ROOT file containing the histograms
    @param variable the variable which defines the x-axis of the histograms
    @param channels the channels for which the histograms are loaded
    """
    signal = dict([(channel, file.GetKey('signal' + key).ReadObj())
                   for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])
    all = dict([(channel, file.GetKey('all' + key).ReadObj())
                for (_, key), channel, file in zip(preCutHistograms, channelNames, files)])

    if any(hist is None for hist in list(signal.values()) + list(all.values())):
        raise RuntimeError('Error while opening ROOT file with preCut Histograms')

    bckgrd = dict([(channel, value.Clone('bckgrd' + channel)) for (channel, value) in all.items()])
    ratio = dict([(channel, value.Clone('ratio' + channel)) for (channel, value) in signal.items()])

    for channel, file in zip(channelNames, files):
        file.cd()
        bckgrd[channel].Add(signal[channel], -1)
        bckgrd[channel].SetTitle('Background')
        bckgrd[channel].Write('', ROOT.TObject.kOverwrite)
        ratio[channel].Divide(bckgrd[channel])
        ratio[channel].SetTitle('Ratio')
        ratio[channel].Write('', ROOT.TObject.kOverwrite)
    return (signal, bckgrd, ratio)


def GetCuts(signal, bckgrd, efficiency, purity, ycut_to_xcuts):
    """
    Calculates the individual cuts on the x-axis for all channels
    @param signal signal histograms of the channels
    @param bckgrd signal histograms of the channels
    @param efficiency the maximal efficiency for the channels in total
    @param purity the minimal purity for the channels in total
    @param ycut_to_xcuts function which calculates xcuts from a given cut on the y-axis
    """

    nSignal = sum([value.GetEntries() for value in list(signal.values())])

    if nSignal == 0:
        channels = [channel for channel in signal.keys()]
        B2WARNING("No signal present in any of these channels: " + (", ".join(channels)))
        return {channel: [0, 0] for channel in signal.keys()}

    def pythonEfficiencyFunc(ycut):
        return sum([GetNumberOfEventsInRange(s, ycut_to_xcuts(channel, ycut[0]))
                    for (channel, s) in signal.items()]) / float(nSignal)

    def pythonPurityFunc(ycut):
        s = sum([GetNumberOfEventsInRange(h, ycut_to_xcuts(channel, ycut[0])) for (channel, h) in signal.items()])
        b = sum([GetNumberOfEventsInRange(h, ycut_to_xcuts(channel, ycut[0])) for (channel, h) in bckgrd.items()])
        if s == 0:
            return 0
        if b == 0:
            return 1
        return s / float(s + b)

    rootEfficiencyFunc = ROOT.TF1('EfficiencyFunction', pythonEfficiencyFunc, 0, 1, 0)
    ycut = -1
    if efficiency is not None:
        ycut = rootEfficiencyFunc.GetX(efficiency, 0.0, 1.0)

    if purity is not None and pythonPurityFunc([ycut]) < purity:
        rootPurityFunc = ROOT.TF1('PurityFunction', pythonPurityFunc, 0, 1, 0)
        ycut = rootPurityFunc.GetX(purity, 0.0, 1.0)

    return {channel: ycut_to_xcuts(channel, ycut) for channel in signal.keys()}


def GetNumberOfEventsInRange(histogram, rangeTuple):
    """
    Calculates the number of events in the given histograms between a and b
    Beware of nasty bug here, if b is exactly the upper edge of a bin, we had to make sure
    that we don't count one bin more than we should!
    @param histograms Histograms
    @param a lower boundary
    @param b upper boundary
    """
    (a, b) = rangeTuple
    low = histogram.FindBin(a)
    if histogram.FindBin(a + histogram.GetBinWidth(low) / 2.0) != low:
        low += 1
    high = histogram.FindBin(b)
    if histogram.FindBin(b - histogram.GetBinWidth(high) / 2.0) != high:
        high -= 1
    return histogram.Integral(low, high) if low < high else 0


def GetNumberOfEvents(histogram):
    """
    Calculates the number of events in the given histograms includeing underflow and overflow bin
    @param histograms Histograms
    """
    return histogram.Integral(0, histogram.GetNbinsX() + 1)


minimum_number_of_particles = 1000


def GetIgnoredChannels(signal, bckgrd, cuts):
    """
    Returns the channels which should be ignored because the cuts are very tight and therefore
    there's not enough statistic for a training.
    @param signal signal histograms of the channels
    @param bckgrd background histograms of the channels
    @param cuts cuts on the x-axis of the channels
    """
    def isIgnored(channel):
        return (GetNumberOfEventsInRange(signal[channel], cuts[channel]) < minimum_number_of_particles or
                GetNumberOfEventsInRange(bckgrd[channel], cuts[channel]) < minimum_number_of_particles)
    return [channel for channel in signal.keys() if isIgnored(channel)]
