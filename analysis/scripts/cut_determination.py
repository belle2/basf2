#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from array import array
import ROOT


class CutDeterminator:

    """
    Determines the optimal cuts for all given decay channels using the
    histograms in the given file
    """

    def __init__(
        self,
        filename,
        channels,
        ratio=0.1,
        ):
        """
        filename is name of the file with the histograms created by the HistMaker module
        channels is a list of channel names
        ratio is the amount of additional signal divide by additional background
            which is just acceptable.
        """

        # # Filename of the ROOT files which stores the histograms
        self.filename = filename
        # # ROOT File which stores the histograms
        self.file = ROOT.TFile(self.filename)
        # # channels for which the cuts are determined
        self.channels = channels
        # # ratio is the amount of additional signal divide by additional
        # # background which is just acceptable.
        self.ratio = ratio

    def getCutOn(self, var):
        """
        Determines the optimal cut values for the given variable.
        """

        # First extract the needed histograms from the file for signal and
        # background for every channel.

        print 'Determine cuts for'
        for c in self.channels:
            print c + '_' + var + '_signal_histogram'

        signal_hists = dict([(c, self.file.Get(c + '_' + var
                            + '_signal_histogram')) for c in self.channels])
        bckgrd_hists = dict([(c, self.file.Get(c + '_' + var
                            + '_background_histogram')) for c in
                            self.channels])

        # Create ratio hists
        ratio_hists = dict([(c, signal_hists[c].Clone(c + '_' + var
                           + '_ratio_histogram')) for c in self.channels])

        result = {}

        for c in channels:
            ratio_hists[c].Divide(bckgrd_hists[c])
            # Position of maximum
            max_pos = ratio_hist[c].GetMaximumBin()
            # DetermineCut positions
            low_cut_pos = 0
            high_cut_pos = size
            for i in range(max_pos + 1, size + 1):
                if ratio_hist[c].GetBinContent(i) < ratio:
                    break
                else:
                    high_cut_pos = i
            for i in reversed(range(0, max_pos)):
                if ratio_hist[c].GetBinContent(i) < ratio:
                    break
                else:
                    low_cut_pos = i
            result[c] = (ratio_hist[c].GetBinCenter(low_cut_pos),
                         ratio_hist[c].GetBinCenter(high_cut_pos))

        return result

    def getCutOn_Slope(self, var):
        """
        This determines the optimal cut values on the prodChildProb variable using
        the same technique as in Belle 1. We create a plot of the signal and
        background amount for different cut values. Then cut values with the same slope for all
        channels are chosen, so a dirty channel is going to get a harder cut then a
        clan channel.
        """

        # First extract the needed histograms from the file for signal and
        # background for every channel.
        signal_hists = dict([(c, self.file.Get(c + '_' + var
                            + '_signal_histogram')) for c in self.channels])
        bckgrd_hists = dict([(c, self.file.Get(c + '_' + var
                            + '_background_histogram')) for c in
                            self.channels])

        result = {}

        # Create slope curves
        c = ROOT.TCanvas('mycanvas', 'Slope Curves - Background vs. Signal')
        multigraph = ROOT.TMultiGraph()

        # Create one curve per channel with a different color and marker shape per
        # channel (-> steered by index)
        for (index, d) in enumerate(channels):
            # TGraph size doesn't accept lists, we have to use pythons array class
            x = array('f')
            y = array('f')
            size = signal_hists[d].GetNbinsX()

            for i in range(0, size):
                x.append(signal_hists[d].Integral(0, i + 1))
                y.append(bckgrd_hists[d].Integral(0, i + 1))

            graph = ROOT.TGraph(size, x, y)
            graph.SetName(d)
            graph.SetTitle(d)
            graph.SetMarkerStyle(21 + index)
            graph.SetFillStyle(0)
            graph.SetLineColor(index)
            graph.SetLineWidth(4)
            graph.SetMarkerColor(index)
            graph.Fit('pol4')
            graph.GetFunction('pol4').SetLineColor(index)
            multigraph.Add(graph)

            # Determine Cut
            # Look for slope probability slope
            cut = 1
            for i in reversed(range(1, size)):
                try:
                    if (y[i] - y[i - 1]) / (x[i] - x[i - 1]) > 1 / self.ratio:
                        cut = i
                        break
                except:
                    cut = i
            result[d] = (1.0 / size * cut, 1)

        # Draw curves
        multigraph.Draw('AP')
        multigraph.GetXaxis().SetTitle('Signal Events')
        multigraph.GetYaxis().SetTitleOffset(1.4)
        multigraph.GetYaxis().SetTitle('Background Events')
        c.BuildLegend()
        c.Update()
        c.SaveAs(self.filename + '.png')
        c.Close()

        return result


