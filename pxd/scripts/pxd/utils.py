#!/usr/bin/python
# -*- coding: utf-8 -*-

"""\
Utilities for PXD calibration and performance study
Author: qingyuan.liu@desy.de
"""

import ROOT
from ROOT import Belle2


# Helper functions for Belle2.VxdID
def get_name(self, separator="_"):
    return f"{self.getLayerNumber()}{separator}{self.getLadderNumber()}{separator}{self.getSensorNumber()}"


def get_pxdid(self):
    return self.getLayerNumber() * 1000 + self.getLadderNumber() * 10 + self.getSensorNumber()


# Helper function for TGraph
def graph_append(self, x, y):
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


def get_sensor_maps(name="MaskedPixels", title="Masked Pixels", ztitle="isMasked", run=0, sensorID_list=sensorID_list):
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


def style():
    """
    ROOT style
    Author: maiko.takahashi@desy.de
    """
    # root style option

    # canvas
    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasBorderSize(10)
    ROOT.gStyle.SetCanvasColor(0)
    ROOT.gStyle.SetCanvasDefH(450)
    ROOT.gStyle.SetCanvasDefW(500)
    ROOT.gStyle.SetCanvasDefX(10)
    ROOT.gStyle.SetCanvasDefY(10)
    # pad
    ROOT.gStyle.SetPadBorderMode(0)
    ROOT.gStyle.SetPadBorderSize(10)
    ROOT.gStyle.SetPadColor(0)
    ROOT.gStyle.SetPadBottomMargin(0.16)
    ROOT.gStyle.SetPadTopMargin(0.10)
    ROOT.gStyle.SetPadLeftMargin(0.17)
    ROOT.gStyle.SetPadRightMargin(0.19)
    ROOT.gStyle.SetPadGridX(1)
    ROOT.gStyle.SetPadGridY(1)
    ROOT.gStyle.SetGridStyle(2)
    ROOT.gStyle.SetGridColor(ROOT.kGray + 1)
    # frame
    ROOT.gStyle.SetFrameFillStyle(0)
    ROOT.gStyle.SetFrameFillColor(0)
    ROOT.gStyle.SetFrameLineColor(1)
    ROOT.gStyle.SetFrameLineStyle(0)
    ROOT.gStyle.SetFrameLineWidth(2)
    ROOT.gStyle.SetFrameBorderMode(0)
    ROOT.gStyle.SetFrameBorderSize(10)
    # axis
    ROOT.gStyle.SetLabelFont(42, "xyz")
    ROOT.gStyle.SetLabelOffset(0.015, "xyz")
    ROOT.gStyle.SetLabelSize(0.048, "xyz")
    ROOT.gStyle.SetNdivisions(505, "xyz")
    ROOT.gStyle.SetTitleFont(42, "xyz")
    ROOT.gStyle.SetTitleSize(0.050, "xyz")
    ROOT.gStyle.SetTitleOffset(1.3, "x")
    ROOT.gStyle.SetTitleOffset(1.2, "y")
    ROOT.gStyle.SetTitleOffset(1.4, "z")
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    # title
    ROOT.gStyle.SetTitleBorderSize(0)
    ROOT.gStyle.SetTitleFillColor(10)
    ROOT.gStyle.SetTitleAlign(12)
    ROOT.gStyle.SetTitleFontSize(0.045)
    ROOT.gStyle.SetTitleX(0.560)
    ROOT.gStyle.SetTitleY(0.860)
    ROOT.gStyle.SetTitleFont(42, "")
    # stat
    ROOT.gStyle.SetStatBorderSize(0)
    ROOT.gStyle.SetStatColor(10)
    ROOT.gStyle.SetStatFont(42)
    ROOT.gStyle.SetStatX(0.94)
    ROOT.gStyle.SetStatY(0.91)
    # histo style
    ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetHistLineWidth(3)
    ROOT.TH2.SetDefaultSumw2()


if __name__ == '__main__':
    style()
