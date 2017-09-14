#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
from array import array
from ROOT import gROOT, gStyle
from ROOT import TFile, TF1, TH1D, TCanvas
from ROOT import TH2D, TGraph, TLine, TBox
from ROOT import kFullCircle, kOpenCircle
from ROOT import kRed, kBlue, kGreen
from ROOT import TGraphErrors
import argparse

nWires = [160, 160, 160, 160, 160, 160, 160, 160,
          160, 160, 160, 160, 160, 160,
          192, 192, 192, 192, 192, 192,
          224, 224, 224, 224, 224, 224,
          256, 256, 256, 256, 256, 256,
          288, 288, 288, 288, 288, 288,
          320, 320, 320, 320, 320, 320,
          352, 352, 352, 352, 352, 352,
          384, 384, 384, 384, 384, 384]

nWiresSum = []
sum = 0
for w in nWires:
    sum = sum + w
    nWiresSum.append(sum)

nWiresSL = [160 * 8, 160 * 6, 192 * 6, 224 * 6, 256 * 6, 288 * 6,
            320 * 6, 352 * 6, 384 * 6]

rfClock = 1.017774
refT0 = 4905.0 * rfClock
refMPV = 80


class DQM():

    """
    DQM class.
    """

    def __init__(self, fname='output.root'):
        """
        call constructor of base class, required.
        """

        super(DQM, self).__init__()

        #: input file name
        self.f = TFile(fname)
        #: plot type (adc/tdc/hit)
        self.type = 'adc'
        #: ADC histograms
        self.histADC = [self.f.Get('h' + str(100000 + i))
                        for i in range(14336)]
        #: TDC histograms
        self.histTDC = [self.f.Get('h' + str(200000 + i))
                        for i in range(14336)]
        #: Hit distribution histograms
        self.histHit = [self.f.Get('h' + str(400000 + i))
                        for i in range(56)]
        #: Line for guide of TDC T0
        self.lines = []
        for h in self.histTDC:
            y = h.GetMaximum()
            line = TLine(refT0, 0, refT0, y)
            line.SetLineColor(2)
            self.lines.append(line)

        #: Line for guide of ADC MPV
        self.lineMPVs = []
        for h in self.histADC:
            y = h.GetMaximum()
            line = TLine(refMPV, 0, refMPV, y)
            line.SetLineColor(2)
            self.lineMPVs.append(line)
        #        self.histADCinLayer = [self.f.Get('h' + str(500000 + i))
        #                               for i in range(56)]

        #: Fitting function for ADC histogram
        self.sig = TF1("sig", "landau", 0, 200)
        #        self.sig = TF1("sig", "landau+expo(3)", 0, 200)
        #        self.sig.SetParameters(300.0,60.0,20.0,1000,-1.0)
        #: Fitting function for TDC histogram
        self.ft0 = TF1("ft0", "[0]+[1]*(exp([2]*([3]-x))/(1+exp(-([4]-x)/[5])))", 2000, 4000)
        self.ft0.SetParameters(0, 10, 0, 0, 3800, 3.)
        self.ft0.FixParameter(0, 0.)
        #: Fit status True if fitting has been finished
        self.m_fitStatus = False
        #: canvas
        self.canvas = TCanvas("canvas", "canvas", 800, 800)
        #: histogram ID
        self.hid = 0
        #: Number of division for canvas
        self.ndiv = 1
        #: MPV values
        self.par = [-1.0 for i in range(14336)]
        #: MPV values w.r.t. layer
        self.parADCinLayer = [-1.0 for i in range(56)]
        #: T0 values
        self.parT0 = [0.0 for i in range(14336)]
        #: histogram
        self.h2 = TH2D("h2d", "MPV of all ADCs", 15000,
                       -500, 14500, 100, 0, 200)
        #: Set stats.
        self.h2.SetStats(0)
        #: Set title of X axis
        self.h2.GetXaxis().SetTitle('Cell ID')
        #: Set title of Y axis
        self.h2.GetYaxis().SetTitle('MPV')
        #: histogram
        self.h2ADCvsLayer = TH2D("h2dl", "MPV for every Layer", 56,
                                 0, 56, 100, 0, 200)
        #: Set stats.
        self.h2ADCvsLayer.SetStats(0)
        #: Set title of X axis
        self.h2ADCvsLayer.GetXaxis().SetTitle('Layer')
        #: Set title of Y axis
        self.h2ADCvsLayer.GetYaxis().SetTitle('MPV (adc count)')
        #: array cell
        self.x = array("d", [i for i in range(14336)])
        #: array layer
        self.l = array("d", [i for i in range(56)])
        #: TGraph
        self.graph = TGraph(len(self.x))
        #: Marker style
        self.graph.SetMarkerStyle(kFullCircle)
        #: Marker size
        self.graph.SetMarkerSize(0.3)

        #: TGraph
        self.graph2 = TGraph(len(self.l))
        #: Marker style
        self.graph2.SetMarkerStyle(kFullCircle)
        #: Marker size
        self.graph2.SetMarkerSize(1.0)
        #: Reference line
        self.line = [TLine(0, 0, 0, 200)]

        x0 = 0
        for i in range(9):
            x0 = x0 + nWiresSL[i]
            line = TLine(x0, 0, x0, 200)
            self.line.append(line)
        for l in self.line:
            l.SetLineColorAlpha(8, 0.5)
            l.SetLineStyle(2)

    def fitADC(self, xmin=8, xmax=200):
        """
        Fit all ADC histgrams.
        """
        if self.m_fitStatus is True:
            print("Fitting for ADC has been already applied.")
        else:

            for (i, h) in enumerate(self.histADC):
                if h.GetEntries() > 0:
                    h.Fit(self.sig, "Q", "", xmin, xmax)
                    self.par[i] = self.sig.GetParameter(1)
            self.m_fitStatus = True

    def getHistID(self, lay=0, wire=0):
        """
        Get Hist ID from (layer, wire) ID.
        Return values : hist ID (cell ID)
        """
        hid = nWiresSum[lay - 1] + wire if lay > 0 else wire
        return hid

    def getLayerWireID(self, hid=0):
        """
        Get (layer, wire) ID from cell ID.
        Return values : [layer, wire]
        """
        if hid < nWiresSum[0]:
            return [0, hid]
        else:
            layer = 0
            wire = 0
            for (i, wsum) in enumerate(nWiresSum):
                diff = hid - wsum
                if diff > 0 and diff < nWires[i]:
                    layer = i + 1
                    wire = diff
                    break
            return [layer, wire]

    def drawADC(self, l=0, w=0):
        """
        Draw ADC histgrams w.r.t (lay,wire).
        """
        i = self.getHistID(l, w)
        for j in range(self.ndiv):
            self.hid = i + j
            self.canvas.cd(j + 1)
            self.histADC[self.hid].Draw()
            self.lineMPVs[self.hid].Draw()

        self.type = 'adc'
        self.canvas.Update()

    def drawTDC(self, l=0, w=0):
        """
        Draw TDC histgrams w.r.t (lay,wire).
        """
        i = self.getHistID(l, w)
        for j in range(self.ndiv):
            self.hid = i + j
            self.canvas.cd(j + 1)
            self.histTDC[self.hid].Draw()
            self.lines[self.hid].Draw()

        self.canvas.Update()
        self.type = 'tdc'

    def drawHit(self, l=0):
        """
        Draw Hit histgrams w.r.t layer ID.
        """
        for j in range(self.ndiv):
            self.hid = l + j
            if self.hid > 55:
                break
            self.canvas.cd(j + 1)
            self.histHit[self.hid].Draw()
        self.canvas.Update()
        self.type = 'hit'

    def drawTDCByCell(self, i):
        """
        Draw TDC histgrams w.r.t cell ID (0-14336).
        """
        for j in range(self.ndiv):
            self.hid = i + j
            self.canvas.cd(j + 1)
            self.histTDC[self.hid].Draw()
            self.lines[self.hid].Draw()

        self.type = 'tdc'
        self.canvas.Update()

    def drawADCByCell(self, i):
        """
        Draw ADC histgrams w.r.t cell ID (0-14336).
        """
        for j in range(self.ndiv):
            self.hid = i + j
            self.canvas.cd(j + 1)
            self.histADC[self.hid].Draw()
            self.lineMPVs[self.hid].Draw()

        self.type = 'adc'
        self.canvas.Update()

    def next(self):
        """
        Show next plots.
        """
        self.hid = self.hid + 1
        if self.type == 'adc':
            self.canvas.Clear('d')
            self.drawADCByCell(self.hid)
        elif self.type == 'tdc':
            self.canvas.Clear('d')
            self.drawTDCByCell(self.hid)
        elif self.type == 'hit':
            self.canvas.Clear('d')
            self.drawHit(self.hid)
        else:
            print('Undefined type: ' + type)

    def div(self, i=1, j=1):
        """
        Divide Tcanvas by (i,j).
        """
        self.canvas.Clear()
        self.canvas.Divide(i, j)
        self.ndiv = i * j

    def mpv(self):
        """
        Show the MPV of ADC distribution
        for all wires.
        """
        self.canvas.Clear()

        for (i, p) in enumerate(self.par):
            self.graph.SetPoint(i, self.x[i], p)

        self.h2.Draw()
        for line in self.line:
            line.Draw()
        self.graph.Draw("P")
        self.canvas.Update()

    def xrange(self, xmin=-500, xmax=14500):
        """
        Set x range of MPV plot.
        """
        self.h2.GetXaxis().SetRangeUser(xmin, xmax)

    def xrangeHit(self, xmin=0, xmax=350):
        """
        Set x range of Hit distributions.
        """
        for h in self.histHit:
            h.GetXaxis().SetRangeUser(xmin, xmax)

    def xrangeTDC(self, xmin=0, xmax=4000):
        """
        Set x range of TDC distributions.
        """
        for h in self.histTDC:
            h.GetXaxis().SetRangeUser(xmin, xmax)

    def xrangeADC(self, xmin=0, xmax=200):
        """
        Set x range of ADC distributions.
        """
        for h in self.histADC:
            h.GetXaxis().SetRangeUser(xmin, xmax)

    def print(self, fname='test.png'):
        """
        Print the current plot.
        """
        self.canvas.Print(fname)

    def ne(self):
        """
        Alias to next()
        """
        self.next()

    def dh(self, l):
        """
        Alias to drawHit()
        """
        self.drawHit(l)

    def da(self, l, w):
        """
        Alias to drawADC()
        """
        self.drawADC(l, w)

    def dt(self, l, w):
        """
        Alias to drawTDC()
        """
        self.drawTDC(l, w)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    args = parser.parse_args()
    dqm = DQM(args.input)
