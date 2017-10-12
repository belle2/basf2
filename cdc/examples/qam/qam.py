#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import ROOT
from ROOT import TFile, TH1D, TH2D, TCanvas, TFile, TChain
from ROOT import TH1F
import argparse
import glob


class QAM():

    """
    QAM class.
    """

    def __init__(self, fname='input.root'):
        """
        call constructor of base class, required.
        """

        super(QAM, self).__init__()

        #: input file name
        self.f = TFile(fname)
        #: Histograms for helix parameters etc...
        self.histHelix = {'ndf': {'all': self.f.Get('h17;1'),
                                  'up': self.f.Get('h00;1'),
                                  'down': self.f.Get('h01;1')},
                          'pval': {'all': self.f.Get('h18;1'),
                                   'up': self.f.Get('h02;1'),
                                   'down': self.f.Get('h03;1')},
                          'd0': {'all': self.f.Get('h11;1'),
                                 'up': self.f.Get('h04;1'),
                                 'down': self.f.Get('h05;1')},
                          'phi0': {'all': self.f.Get('h12;1'),
                                   'up': self.f.Get('h06;1'),
                                   'down': self.f.Get('h07;1')},
                          'omega': {'all': self.f.Get('h13;1'),
                                    'up': self.f.Get('h08;1'),
                                    'down': self.f.Get('h09;1')},
                          'z0': {'all': self.f.Get('h14;1'),
                                 'up': self.f.Get('h0a;1'),
                                 'down': self.f.Get('h0b;1')},
                          'tanl': {'all': self.f.Get('h15;1'),
                                   'up': self.f.Get('h0c;1'),
                                   'down': self.f.Get('h0d;1')},
                          'pt': {'all': self.f.Get('h16;1'),
                                 'up': self.f.Get('h0e;1'),
                                 'down': self.f.Get('h0f;1')}
                          }

        #: Resolution histograms
        self.histReso = {'d0': self.f.Get('h1;1'),
                         'phi0': self.f.Get('h2;1'),
                         'omega': self.f.Get('h3;1'),
                         'z0': self.f.Get('h4;1'),
                         'tanl': self.f.Get('h5;1'),
                         'pt': self.f.Get('h6;1')}

        #: Pull histograms
        self.histPull = {'d0': self.f.Get('h21;1'),
                         'phi0': self.f.Get('h22;1'),
                         'omega': self.f.Get('h23;1'),
                         'z0': self.f.Get('h24;1'),
                         'tanl': self.f.Get('h25;1')}

        #: Graph, resolution as a function of pt
        self.graphPt = {'d0': self.f.Get('Graph;2'),
                        'phi0': self.f.Get('Graph;3'),
                        'omega': self.f.Get('Graph;4'),
                        'z0': self.f.Get('Graph;5'),
                        'pt': self.f.Get('Graph;1')}
        #: canvas
        self.canvas = TCanvas("canvas", "canvas", 800, 800)
        #: Number of division for canvas
        self.ndiv = 1
        #: Index of canvas position
        self.index = 1

    def pull(self, key='d0'):
        '''
        Plot pull distribution
        '''
        self.canvas.cd(self.index)
        self.histPull[key].Draw()
        self.canvas.Update()
        self.index = self.index + 1

    def getMean(self, key='pt'):
        '''
        Getter for mean of helix parameter.
        '''
        h = self.histReso[key]
        m = h.GetMean()
        dm = h.GetMeanError()
        return [m, dm]

    def getRms(self, key='pt'):
        '''
        Getter for rms of helix parameter.
        '''
        h = self.histReso[key]
        s = h.GetRMS()
        ds = h.GetRMSError()
        return [s, ds]

    def graph(self, key='pt'):
        '''
        Plot graph of resolution as a function of Pt.
        '''
        self.canvas.cd(self.index)
        self.graphPt[key].Draw('AP')
        self.canvas.Update()
        self.index = self.index + 1

    def reso(self, key='pt'):
        '''
        Plot resolution histogram.
        '''
        self.canvas.cd(self.index)
        self.histReso[key].Draw()
        self.canvas.Update()
        self.index = self.index + 1

    def draw(self, key='pt', option='all', gopt=''):
        '''
        Plot histogram of helix parameters etc..
        '''
        self.canvas.cd(self.index)
        self.histHelix[key][option].Draw(gopt)
        self.canvas.Update()
        self.index = self.index + 1

    def print(self, fname='test.png'):
        """
        Print the current plot.
        """
        self.canvas.Print(fname)

    def div(self, i=1, j=1):
        """
        Divide Tcanvas by (i,j).
        """
        self.canvas.Clear()
        self.canvas.Divide(i, j)
        self.ndiv = i * j
        self.index = 1


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    args = parser.parse_args()
    qam = QAM(args.input)
