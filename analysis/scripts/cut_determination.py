#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

import ROOT


class ChannelCut:
    """
    Represents a cut on a specific channel
    """

    def __init__(self, file, channel):
        """
        Determines the cut on a specific channel
        file - the ROOT TFile where the histograms are stored
        channel - the channel name as string
        """
        ## Distribution of invariant mass for signal events
        self.signal = file.Get(channel + '_M_signal_histogram')
        ## Distribution of invariant mass for background events
        self.bckgrd = file.Get(channel + '_M_background_histogram')
        ## Signal to Background Ration for invariant mass
        self.ratio = self.signal.Clone(channel + '_M_ratio_histogram')
        self.ratio.Divide(self.bckgrd)
        self.ratio.Write('', ROOT.TObject.kOverwrite)
        ## Spline fit of Signal to Backgruond Ratio
        self.spline = ROOT.TSpline3(self.ratio)
        ## Position of maximum of Signal to Background Ratio
        ## This should coincidence with the nominal mass of the particle
        self.maxpos = self.signal.GetBinCenter(self.signal.GetMaximumBin())
        ## ROOT TF1 function wrapping the spline fit
        self.func = ROOT.TF1(str(id(channel)) + '_M_func', lambda x: \
                             self.spline.Eval(x[0]), 0, 100, 0)

    def getCut(self, d):
        """
        Returns cut positions at x-axis for a cut d on y-axis
        """
        return (self.func.GetX(d, 0, self.maxpos), self.func.GetX(d,
                self.maxpos, 100))

    def getNEvent(self):
        """
        Returns total number of recorded events in this channel.
        """
        return self.signal.Integral(0, self.signal.GetNbinsX()) \
            + self.bckgrd.Integral(0, self.bckgrd.GetNbinsX())

    def getNEventCut(self, d):
        """
        Returns total number of events which surviving the cut d
        """
        (a, b) = self.getCut(d)
        return self.signal.Integral(self.signal.FindBin(a),
                                    self.signal.FindBin(b)) \
            + self.bckgrd.Integral(self.bckgrd.FindBin(a),
                                   self.bckgrd.FindBin(b))


class GlobalCut:
    """
    Represent a cut on a specific particle.
    """

    def __init__(self, file, channels):
        """
        channels is a list of channel names
        """

        ## List of ChannelCut objects for each channel
        self.channel_cuts = [ChannelCut(file, channel) for channel in channels]

    def getCuts(self, d):
        """
        Returns dictionary of cut positions on the x-axis for a cut d on the
        y-axis
        """
        return dict([(c.channel, c.getCut(d)) for c in self.channel_cuts])

    def getNEvent(self):
        """
        Returns total number of recorded events in all channels
        """
        return sum([c.getNEvent() for c in self.channel_cuts])

    def __call__(self, d):
        """
        Function which is used by TF1, to implement mass_cut function.
        Returns the total number of events which survive a cut d
        """
        return sum([c.getNEventCut(d[0]) for c in self.channel_cuts])


def getCutOnMass(percentage, filename, channels):
    """
    Calculates a cut on all channels from the histograms in filename so only
    percentage of the events survive the cut.
    """
    file = ROOT.TFile(filename, 'UPDATE')
    global_cuts = GlobalCut(file, channels)
    func = ROOT.TF1(filename + '_mass_cut_func', global_cuts, 0, 1, 0)
    nevent = percentage * global_cuts.getNEvent()
    d = func.GetX(nevent, 0.0, 1.0)
    return global_cuts.getCuts(d)
