#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

import ROOT


class ChannelCut:

    def __init__(self, file, channel):
        self.channel = channel
        self.signal = file.Get(channel + '_M_signal_histogram')
        self.bckgrd = file.Get(channel + '_M_background_histogram')

        self.ratio = self.signal.Clone(channel + '_M_ratio_histogram')
        self.ratio.Divide(self.bckgrd)
        self.ratio.Write('', ROOT.TObject.kOverwrite)
        self.spline = ROOT.TSpline3(self.ratio)

        self.maxpos = self.signal.GetBinCenter(self.signal.GetMaximumBin())
        self.func = ROOT.TF1(str(id(channel)) + '_M_func', lambda x: \
                             self.spline.Eval(x[0]), 0, 100, 0)

    def getCut(self, d):
        return (self.func.GetX(d, 0, self.maxpos), self.func.GetX(d,
                self.maxpos, 100))

    def getNEvent(self):
        return self.signal.Integral(0, self.signal.GetNbinsX()) \
            + self.bckgrd.Integral(0, self.bckgrd.GetNbinsX())

    def getNEventCut(self, d):
        (a, b) = self.getCut(d)
        return self.signal.Integral(self.signal.FindBin(a),
                                    self.signal.FindBin(b)) \
            + self.bckgrd.Integral(self.bckgrd.FindBin(a),
                                   self.bckgrd.FindBin(b))


class GlobalCut:

    def __init__(self, file, channels):
        """
        channels is a list of channel names
        """

        self.channel_cuts = [ChannelCut(file, channel) for channel in channels]

    def getCuts(self, d):
        return dict([(c.channel, c.getCut(d)) for c in self.channel_cuts])

    def getNEvent(self):
        return sum([c.getNEvent() for c in self.channel_cuts])

    def __call__(self, d):
        return d[0]


        # return sum([c.getNEventCut(d[0]) for c in self.channel_cuts])


def getCutOnMass(percentage, filename, channels):
    file = ROOT.TFile(filename, 'UPDATE')
    global_cuts = GlobalCut(file, channels)
    func = ROOT.TF1(filename + '_mass_cut_func', global_cuts, 0, 1, 0)
    nevent = percentage * global_cuts.getNEvent()
    d = func.GetX(nevent, 0.0, 1.0)
    return global_cuts.getCuts(d)


