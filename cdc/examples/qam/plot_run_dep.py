#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import ROOT
from ROOT import TFile, TH1D, TH2D, TCanvas, TFile, TChain
from ROOT import TH1F, TGraph, TGraphErrors
import argparse
import glob
from qam import QAM
from array import array
x = array('d')
dx = array('d')

d0 = array('d')
dd0 = array('d')
z0 = array('d')
dz0 = array('d')
pt = array('d')
dpt = array('d')

# files = glob.glob('output_aug/qam.0*.root')
files = glob.glob('/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/build-2017-08-21/qam.0001.*.root')

if files is None:
    print('No files are found')
    exit(1)

for f in files:
    run = int(f.split('.')[2])
    qam = QAM(f)
    rmsPt = qam.getRms('pt')
    rmsD0 = qam.getRms('d0')
    rmsZ0 = qam.getRms('z0')
    x.append(run)
    dx.append(0.0)
    d0.append(rmsD0[0])
    dd0.append(rmsD0[1])
    z0.append(rmsZ0[0])
    dz0.append(rmsZ0[1])
    pt.append(rmsPt[0])
    dpt.append(rmsPt[1])


can = TCanvas('can', 'can', 800, 800)
dgr = TGraphErrors(len(x), x, d0, dx, dd0)
dgr.SetMarkerColorAlpha(2, 0.5)
dgr.SetMarkerStyle(20)
dgr.SetTitle('Run dependence')
dgr.GetXaxis().SetTitle('run number')
dgr.GetYaxis().SetTitle('RMS #Delta d_{0}')
dgr.Draw('AP')

can.Print('d0.png')

zgr = TGraphErrors(len(x), x, z0, dx, dz0)
zgr.SetMarkerColorAlpha(2, 0.5)
zgr.SetMarkerStyle(20)
zgr.SetTitle('Run dependence')
zgr.GetXaxis().SetTitle('run number')
zgr.GetYaxis().SetTitle('RMS #Delta z_{0}')
zgr.Draw('AP')

can.Print('z0.png')

pgr = TGraphErrors(len(x), x, pt, dx, dpt)
pgr.SetMarkerColorAlpha(2, 0.5)
pgr.SetMarkerStyle(20)
pgr.SetTitle('Run dependence')
pgr.GetXaxis().SetTitle('run number')
pgr.GetYaxis().SetTitle('RMS #Delta P_{t}')
pgr.Draw('AP')

can.Print('pt.png')

'''
import numpy as np
import matplotlib.pylab as plt
xx = np.array(x)
yy = np.array(y)

plt.plot(x,y,marker='o', color='r', alpha=0.4)
plt.show()
'''
