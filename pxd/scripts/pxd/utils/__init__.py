#!/usr/bin/python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
Utilities for PXD calibration and performance study
"""

import ROOT
from ROOT import Belle2
import pandas as pd
import numpy as np


# Helper functions for Belle2.VxdID
def get_name(self, separator="_"):
    """
    Get PXD module name with a specific format
    Parameters:
      self: Belle2.VxdID
      separator (str): separator between different (layer/ladder/sensor) numbers.
    Return:
      PXD module name, e.g., 1_1_1
    """
    return f"{self.getLayerNumber()}{separator}{self.getLadderNumber()}{separator}{self.getSensorNumber()}"


def get_pxdid(self):
    """
    Get PXD module id: layer_id * 1000 + ladder_id * 10 + sensor_id
    Parameters:
      self: Belle2.VxdID
    Return:
      PXD module id, e.g., 1011
    """
    return self.getLayerNumber() * 1000 + self.getLadderNumber() * 10 + self.getSensorNumber()


# Helper function for TGraph
def graph_append(self, x, y):
    """
    Append a point to TGraph
    Parameters:
      self: ROOT.TGraph
      x (float): x coordinate of the point
      y (float): y coordinate of the point
    """
    self.SetPoint(self.GetN(), x, y)


# Extend Belle2.VxdID
Belle2.VxdID.get_name = get_name
Belle2.VxdID.get_pxdid = get_pxdid
# Extend ROOT classes
ROOT.TGraph.append = graph_append

# Sensor id list
sensor_labels = [
    "1.1.1", "1.1.2",
    "1.2.1", "1.2.2",
    "1.3.1", "1.3.2",
    "1.4.1", "1.4.2",
    "1.5.1", "1.5.2",
    "1.6.1", "1.6.2",
    "1.7.1", "1.7.2",
    "1.8.1", "1.8.2",
    "2.4.1", "2.4.2",
    "2.5.1", "2.5.2"]
sensorID_list = tuple([Belle2.VxdID(label) for label in sensor_labels])
nsensors = len(sensorID_list)

# Sensor matrix
nUCells = 250
nVCells = 768
nPixels = 192000

# Colors and styles for ROOT
colors = tuple([
    ROOT.kRed + 1, ROOT.kOrange + 1, ROOT.kYellow - 3, ROOT.kSpring + 5, ROOT.kGreen + 3,
    ROOT.kCyan - 6, ROOT.kAzure - 5, ROOT.kAzure - 6, ROOT.kBlue + 3, ROOT.kViolet - 1])
#: update the plot_configs with sensor specific styles
plot_configs = {
    "LineWidth": 4,
    "yTitleOffset": 0.9
}


def get_sensor_graphs(ytitle=""):
    """
    Create TGraphs and related TLegends
    Parameters:
      ytitle (str): Title of the y-axis
    Return:
      A dictionary using sensorID as key and TGraph as value. A special key "TLegends"
      is used for the list of TLegend objects for drawing.
    """
    graphs = {}
    legs = []
    # Create TLegends
    xlegl = [0.60, 0.57, 0.78, 0.75]
    xlegr = [0.72, 0.65, 0.90, 0.83]
    for i in range(4):
        """
        Display format (content in a TLegend is bracketed):
          (marker1 /) (marker2 label1/label2) (marker3 /) (markter4 label3/label4)
        """
        legs.append(ROOT.TLegend(xlegl[i], 0.62, xlegr[i], 0.85))
        legs[i].SetFillStyle(0)
        legs[i].SetBorderSize(0)
        legs[i].SetTextFont(43)
        legs[i].SetTextSize(18)
    # Create TGraphs and set TLegend for each sensor
    for i, sensorID in enumerate(sensorID_list):
        agraph = ROOT.TGraph()
        # agraph.SetTitle(sensorID.get_name())
        agraph.SetName(sensorID.get_name())
        agraph.GetXaxis().SetTitle('run #')
        agraph.GetYaxis().SetTitle(ytitle)
        agraph.GetYaxis().SetTitleOffset(0.9)
        agraph.SetLineColor(colors[int(i / 2)])
        agraph.SetLineWidth(4)
        agraph.SetMarkerColor(colors[int(i / 2)])
        agraph.SetMarkerStyle(26 if i % 2 else 24)
        graphs[sensorID.getID()] = agraph
        #  Set TLegend
        i1 = 0
        i2 = 1
        if (i > 9):
            i1 = 2
            i2 = 3
        if (i % 2):
            legs[i1].AddEntry(agraph, sensor_labels[i - 1] + ' / ' + sensor_labels[i], 'p')
        else:
            legs[i2].AddEntry(agraph, ' / ', 'p')
    graphs["TLegends"] = legs

    return graphs


def get_sensor_maps(
    name="MaskedPixels", title="Masked Pixels", ztitle="isMasked", run=0,
    nUCells=nUCells, nVCells=nVCells, sensorID_list=sensorID_list
):
    """
    Create TH2F objects for saving pixel map
    Parameters:
      name (str): Name of a histogram
      title (str): Title
      ztitle (str): Label for z-axis
      run (int): run number of the map, to be appended into the title
      sensorID_list (list): List of sensorID objects for which histograms are created.
    Return:
      A dictionary using sensorID as key and TH2F as value.
    """
    hists = {}
    for sensorID in sensorID_list:
        hname = name + "_{}_run_{:d}".format(sensorID.get_name(), run)
        htitle = title + " Sensor={:d} Run={:d}".format(sensorID.get_pxdid(), run)
        h2 = ROOT.TH2F(hname, htitle, nUCells, 0, nUCells, nVCells, 0, nVCells)
        h2.GetXaxis().SetTitle("uCell")
        h2.GetYaxis().SetTitle("vCell")
        h2.GetZaxis().SetTitle(ztitle)
        h2.SetStats(0)
        hists[sensorID.getID()] = h2
    return hists


def df_calculate_eff(df, num="nTrackClusters", den="nTrackPoints", output_var="eff"):
    """
    Efficiency calculation with asymmetric errors for pandas DataFrame
    Parameters:
      df: pandas.DataFrame
      num (str): column used as the numerator
      den (str): column used as the denominator
      output_var (str): column for saving efficiency
    """
    nums = df[num].to_numpy()
    dens = df[den].to_numpy()
    from root_numpy import array2hist
    nBins = len(nums)
    assert len(nums) == len(dens), "len(numerators) != len(denominators)"
    h_num = ROOT.TH1I("h_num", "h_num", nBins, 0, nBins)
    h_den = ROOT.TH1I("h_den", "h_den", nBins, 0, nBins)
    array2hist(nums, h_num)
    array2hist(dens, h_den)
    h_eff = ROOT.TEfficiency(h_num, h_den)
    df[output_var] = df[num]/df[den]
    df[output_var+"_err_low"] = [h_eff.GetEfficiencyErrorLow(i+1) for i in range(nBins)]
    df[output_var+"_err_up"] = [h_eff.GetEfficiencyErrorUp(i+1) for i in range(nBins)]


def getH1Sigma68(h1, fill_random=False):
    """
    Helper function to get sigma68 from TH1
    Parameters:
      h1 (ROOT.TH1): TH1 object from ROOT
      fill_random (bool): Flag to call TH1.FillRandom
    Return:
      sigma68
    """
    qs = np.array([0., 0.])
    probs = np.array([0.16, 0.84])
    if fill_random:
        h1tmp = h1.Clone()
        h1tmp.Reset()
        h1tmp.FillRandom(h1, int(h1.GetEffectiveEntries()))
        h1tmp.GetQuantiles(2, qs, probs)
    else:
        h1.GetQuantiles(2, qs, probs)
    return (qs[1]-qs[0])


def getH1Sigma68WithError(h1, n=300):
    """
    Helper function to get sigma68 and its error using TH1.FillRandom
    Parameters:
      h1 (ROOT.TH1): TH1 object from ROOT
      n (int): number of randomly generated histograms for the estimation
    Return:
      estimatd sigma68 and its standard deviation
    """
    results = np.array([getH1Sigma68(h1, fill_random=True) for i in range(n)])
    # return results.mean(), results.std()
    return getH1Sigma68(h1), results.std()


def getSigma68(array):
    """
    Helper function to get sigma68 and its error using numpy.array
    Parameters:
      array (numpy.array): target array
    Return:
      estimatd sigma68
    """
    q1 = np.quantile(array, 0.16, axis=0)
    q2 = np.quantile(array, 0.84, axis=0)
    return (q2-q1)  # /2*1e4  # cm to um


def getSigma68WithError(array, n=300):
    """
    Helper function to get sigma68 and its error using numpy.array
    Parameters:
      array (numpy.array): target array
      n (int): number of bootstrap drawings
    Return:
      estimatd sigma68 and its standard deviation
    """
    bs = np.random.choice(array, (array.shape[0], n))  # bootstrap resampling
    results = getSigma68(bs)
    # return results.mean(), results.std()
    return getSigma68(a), results.std()


# Extend pandas.DataFrame
pd.DataFrame.calculate_eff = df_calculate_eff

# latex
latex_l = ROOT.TLatex()  # left aligned
latex_l.SetTextFont(43)
latex_l.SetNDC()
latex_l.SetTextSize(24)
latex_ls = ROOT.TLatex()  # left aligned small
latex_ls.SetTextFont(43)
latex_ls.SetNDC()
latex_ls.SetTextSize(19)
latex_r = ROOT.TLatex()  # right aligned
latex_r.SetTextFont(43)
latex_r.SetNDC()
latex_r.SetTextSize(24)
latex_r.SetTextAlign(31)
