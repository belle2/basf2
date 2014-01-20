#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from array import array
import ROOT


def determine(filename, channels):
    """
    Determines the optimal cuts for all given decay channels using the
    histograms in the given file
    filename is name of the file with the histograms created by the HistMaker module
    channels is a list of channel names
    """

    # First we open the given .root file
    file = ROOT.TFile(filename)

    # Extract cuts on different variable from the histograms stored in the file
    cut_maps = []
    cut_maps.append(determine_prodChildProb_cut(file, channels, filename))
    cut_maps.append(determine_invariantMass_cut(file, channels))

    # Now we need to convert the result into a map from channelname -> map of
    # cuts from variable name -> cut range, this result can later be used as
    # input for the cutsOnProduct parameter of the ParticleCombiner module.
    result = {}
    for var in cut_maps:
        for (channel, cuts) in var.iteritems():
            result.setdefault(channel, {})
            result[channel].update(cuts)
    return result


def determine_invariantMass_cut(file, channels):
    """
    This determines the optimal cut values on the invariant mass
    """

    # First extract the needed histograms from the file. Here we need the
    # distribution of prodChildProb for signal and background for every channel.
    signal_hists = {}
    for d in channels:
        signal_hists[d] = file.Get(d + '_M_signal_histogram')
    background_hists = {}
    for d in channels:
        background_hists[d] = file.Get(str(d) + '_M_background_histogram')

    # TODO determine cut
    result = {}
    for d in channels:
        result[d] = {'M': (0.0, 100.0)}

    return result


def determine_prodChildProb_cut(file, channels, filename):
    """
    This determines the optimal cut values on the prodChildProb variable using
    the same technique as in Belle 1. We create a plot of the signal and
    background amount for different cut values. Then cut values with the same slope for all
    channels are chosen, so a dirty channel is going to get a harder cut then a
    clan channel.
    """

    probability_slope = 10

    # First extract the needed histograms from the file. Here we need the
    # distribution of prodChildProb for signal and background for every channel.
    signal_hists = {}
    for d in channels:
        signal_hists[d] = file.Get(d + '_prodChildProb_signal_histogram')
    background_hists = {}
    for d in channels:
        background_hists[d] = file.Get(str(d)
                                       + '_prodChildProb_background_histogram')

    result = {}

    # Create slope curves
    c = ROOT.TCanvas('mycanvas', 'Slope Curves - Background vs. Signal')
    multigraph = ROOT.TMultiGraph()

    # Create one curve per channel with a different color and marker shape per
    # channel (-> steered by color)
    color = 0
    for d in channels:
        color += 1

        # TGraph size doesn't accept lists, we have to use pythons array class
        x = array('f')
        y = array('f')
        size = signal_hists[d].GetNbinsX()

        for i in range(0, size):
            x.append(signal_hists[d].Integral(0, i + 1))
            y.append(background_hists[d].Integral(0, i + 1))

        graph = ROOT.TGraph(size, x, y)
        graph.SetName(d)
        graph.SetTitle(d)
        graph.SetMarkerStyle(21 + color)
        graph.SetFillStyle(0)
        graph.SetLineColor(color)
        graph.SetLineWidth(4)
        graph.SetMarkerColor(color)
        graph.Fit('pol4')
        graph.GetFunction('pol4').SetLineColor(color)
        multigraph.Add(graph)

        # Determine Cut
        # Look for slope probability slope
        cut = 1
        for i in reversed(range(1, size)):
            try:
                if (y[i] - y[i - 1]) / (x[i] - x[i - 1]) > probability_slope:
                    cut = i
                    break
            except:
                cut = i
        result[d] = {'SignalProbability': [1.0 / size * cut, 1]}
        print d, cut

    # Draw curves
    multigraph.Draw('AP')
    multigraph.GetXaxis().SetTitle('Signal Events')
    multigraph.GetYaxis().SetTitleOffset(1.4)
    multigraph.GetYaxis().SetTitle('Background Events')
    c.BuildLegend()
    c.Update()
    c.SaveAs(filename + '.png')
    c.Close()

    return result


