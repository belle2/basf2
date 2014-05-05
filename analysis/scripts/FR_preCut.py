#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

from FR_utility import createHash
from basf2 import *
import modularAnalysis
import pdg
import ROOT
import os
import hashlib


def PreCutMassDetermination(name, pdgcode, channels, preCut_Histograms, efficiency):
    """
    Determines the PreCuts for all the channels of a particle. The cuts are chosen as follows:
        1. The maximum of every signal invariant mass preCut_Histogram is isolated
        2. The ratio of signal/background invariant mass preCut_Histogram is calculated and fitted with splines
        3. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is garantueed.
        4. The cut on the y-axis is converted to a minimum and maximum cut on the x-axis for every channel.
        @param name name of the particle
        @param pdgcode pdgcode of the particle
        @param channels list of the names of all the channels
        @param preCut_Histograms filenames of the histogram files created for every channel by PreCutDistribution
        @param signal efficiency for this particle
    """

    files = [ROOT.TFile(filename, 'UPDATE') for filename in preCut_Histograms]
    signal, bckgrd, ratio = LoadHistogramsFromFiles(files, 'M', channels)
    maxima = GetPositionsOfMaxima(signal)
    splines = FitSplineFunctions(ratio)

    def ycut_to_xcuts(channel, cut):
        return (splines[channel].GetX(cut, 0, maxima[channel]), splines[channel].GetX(cut, maxima[channel], 100))
    cuts = GetCuts(signal, efficiency, ycut_to_xcuts)

    result = {'PreCut_' + channel: {'variable': 'MassCut', 'range': cut} for (channel, cut) in cuts.iteritems()}
    result.update({'IsIgnored_' + channel: False for channel in channels})
    for ignoredChannel in GetIgnoredChannels(signal, bckgrd, cuts):
        result['IsIgnored_' + ignoredChannel] = True
        B2WARNING("Ignoring channel " + ignoredChannel + "!")
    result['Hash'] = createHash(name, pdgcode, channels, preCut_Histograms, efficiency)
    return result


def PreCutProbDetermination(name, pdgcode, channels, preCut_Histograms, efficiency):
    """
    Determines the PreCuts for all the channels of a particle. The cuts are chosen as follows:
        1. The ratio of signal/background child probability product preCut_Histogram is calculated and fitted with splines
        2. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is garantueed.
        3. The cut on the y-axis is converted to a minimum cut on the x-axis for every channel.
        @param name name of the particle
        @param pdgcode pdgcode of the particle
        @param channels list of the names of all the channels
        @param preCut_Histograms filenames of the histogram files created for every channel by PreCutDistribution
        @param efficiency signal efficiency for this particle
    """

    files = [ROOT.TFile(filename, 'UPDATE') for filename in preCut_Histograms]
    signal, bckgrd, ratio = LoadHistogramsFromFiles(files, 'prodChildProb', channels)
    splines = FitSplineFunctions(ratio)

    def ycut_to_xcuts(channel, cut):
        return (splines[channel].GetX(cut, 0, 1), 1)
    cuts = GetCuts(signal, efficiency, ycut_to_xcuts)

    result = {'PreCut_' + channel: {'variable': 'cutsOnProduct', 'range': ('SignalProbability',) + cut} for (channel, cut) in cuts.iteritems()}
    result.update({'IsIgnored_' + channel: False for channel in channels})
    for ignoredChannel in GetIgnoredChannels(signal, bckgrd, cuts):
        result['IsIgnored_' + ignoredChannel] = True
        B2WARNING("Ignoring channel " + ignoredChannel + "!")
    result['Hash'] = createHash(name, pdgcode, channels, preCut_Histogram, efficiency)
    return result


def LoadHistogramsFromFiles(files, variable, channels):
    """
    Load for every channel the signal and backgrund histogram of a variable from given ROOT file and
    calculates the ratio S/B histogram.
    @param filename filename of the ROOT file containing the histograms
    @param variable the variable which defines the x-axis of the histograms
    @param channels the channels for which the histograms are loaded
    """
    signal = dict([(channel, file.Get('{channel}_{variable}_signal_histogram'.format(channel=channel, variable=variable))) for channel, file in zip(channels, files)])
    bckgrd = dict([(channel, file.Get('{channel}_{variable}_background_histogram'.format(channel=channel, variable=variable))) for channel, file in zip(channels, files)])
    ratio = dict([(channel, value.Clone('{channel}_{variable}_ratio_histogram'.format(channel=channel, variable=variable))) for (channel, value) in signal.iteritems()])

    if not all(signal.values() + bckgrd.values() + ratio.values()):
        raise RuntimeError('Error while opening ROOT file with preCut Histograms')

    for channel in channels:
        ratio[channel].Divide(bckgrd[channel])
        ratio[channel].Write('', ROOT.TObject.kOverwrite)
    return (signal, bckgrd, ratio)


def GetPositionsOfMaxima(histograms):
    """
    Calculates the Positions of the Maxima of the given histograms
    @param histograms Histograms
    """
    return dict([(channel, value.GetBinCenter(value.GetMaximumBin())) for (channel, value) in histograms.iteritems()])


def FitSplineFunctions(histograms):
    """
    Fits Spline Functions to the given histograms
    @param histograms Histograms
    """
    splines = dict([(channel, ROOT.TSpline3(value)) for (channel, value) in histograms.iteritems()])
    return dict([(channel, ROOT.TF1(hashlib.sha1(channel).hexdigest() + '_func', lambda x, spline=spline: spline.Eval(x[0]), 0, 100, 0)) for (channel, spline) in splines.iteritems()])


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


def CreatePreCutMassHistogram(path, name, particle, daughterLists):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of every channel (signal/background)
    for a given particle, before any intermediate cuts are applied. For now this class uses the ParticleCombiner and HistMaker
    modules to generate these distributions, later we should join these modules together into one fast PreCutHistogram module.
        @param path the basf2 path
        @param name of the channel
        @param particle object for which this histogram is created
        @param daughterLists all particleLists of all the daughter particles in all channels
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = createHash(name, particle.name, daughterLists)
    filename = 'Reconstruction_{pname}_{cname}_{hash}.root'.format(pname=particle.name, cname=name, hash=hash)
    if os.path.isfile(filename):
        return {'PreCutHistogram_' + name: filename}
    else:
        # Combine all the particles according to the decay channels
        mass = pdg.get(pdg.from_name(particle.name)).Mass()
        pmake = register_module('MCDecayHistMaker')
        pmake.set_name('DecayHistMaker_' + name)
        pmake.param('fileName', filename)
        pmake.param('PDG', pdg.from_name(particle.name))
        pmake.param('inputListNames', daughterLists)
        pmake.param('histParams', (100, mass - mass * 3.0 / 4.0, mass + mass * 2))
        path.add_module(pmake)
    return {}


def CreatePreCutProbHistogram(path, particle, name, daughterLists, daughterSignalProbabilities):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of every channel (signal/background)
    for a given particle, before any intermediate cuts are applied. For now this class uses the ParticleCombiner and HistMaker
    modules to generate these distributions, later we should join these modules together into one fast PreCutHistogram module.
        @param path the basf2 path
        @param particle object for which this histogram is created
        @param name of the channel
        @param daughterLists all particleLists of all the daughter particles in all channels
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = createHash(name, particle.name, daughterLists, daughterSignalProbabilities)
    filename = 'Reconstruction_{pname}_{cname}_{hash}.root'.format(pname=particle.name, cname=name, hash=hash)
    if os.path.isfile(filename):
        return {'PreCutHistogram_' + particle.name: filename}
    else:
        raise NotImplemented
    return {}


def PrintPreCuts(name, channels, preCuts, ignoredChannels):
    print 'PreCuts for {name}'.format(name=name)
    for (channel, preCut, isIgnored) in zip(channels, preCuts, ignoredChannels):
        if isIgnored:
            print 'Channel {channel} is ignored due to low statistics'.format(channel=channel)
        else:
            print 'Channel {channel} has cut on {variable} with range {low} to {high}'.format(channel=channel, variable=preCut['variable'], low=preCut['range'][0], high=preCut['range'][1])
    return {}
