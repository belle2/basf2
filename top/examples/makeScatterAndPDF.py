import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle  # $matplotlib/patches.py

import ROOT
ROOT.gROOT.SetBatch(1)  # something's trying to open xwindows
import os


def Gaussian(mean, width, norm):
    """Make a Gaussian function"""
    def g(x):
        return norm / width / np.sqrt(2 * np.pi) * np.exp(-((mean - x) / width)**2)
    return g


def PDF(listOfGaussians, x):
    """Remake the pdf for a single pixel (a sum of many Gaussians)"""
    return sum(g(x) for g in listOfGaussians)


xbins = np.linspace(0, 512, 513)
ybins = np.linspace(-5, 80, 201)
xcentres = xbins[0:-1] + 0.5 * (xbins[1:] - xbins[:-1])
ycentres = ybins[0:-1] + 0.5 * (ybins[1:] - ybins[:-1])
X, Y = np.meshgrid(xcentres, ycentres)


def makeGaussianPlot(infile):
    f = ROOT.TFile.Open(infile)
    t = f.Get("tree")
    t.GetEntry(0)
    x = t.TOPPDFCollections[0]
    # get the pdf, arbitrarily at 250
    pxData = x.m_data[13][250]
    lOG = []
    for peak, width, norm in pxData:
        lOG.append(Gaussian(peak, width, norm))
    f.Close()
    # returns the value of the Gaussian for each time bin
    return np.array([PDF(lOG, timebin) for timebin in ycentres])


def makePcolor(infile):
    f = ROOT.TFile.Open(infile)
    t = f.Get("tree")
    t.GetEntry(0)
    x = t.TOPPDFCollections[0]
    Z = np.zeros_like(X, dtype=np.float32)
    for pixel, pxData in enumerate(x.m_data[13]):
        if pixel >= len(xcentres):
            break
        lOG = []  # list of Gaussians
        for peak, width, norm in pxData:
            lOG.append(Gaussian(peak, width, norm))
        # returns the value of the Gaussian for each time bin
        Z[:, pixel] = np.array([PDF(lOG, timebin) for timebin in ycentres])
    f.Close()
    return Z

Z1 = makePcolor("Movie/Momentum/pdf_momentum_scan_pid211_step_092.root")
Z2 = makePcolor("Movie/Momentum/pdf_momentum_scan_pid321_step_092.root")
plt.pcolor(X, Y, Z1, alpha=0.9, cmap="Blues")
plt.savefig("pion_pcolor.pdf")
plt.clf()
plt.pcolor(X, Y, Z2, alpha=0.9, cmap="Oranges")
plt.savefig("kaon_pcolor.pdf")
plt.clf()
plt.pcolor(X, Y, Z1, alpha=0.9, cmap="Blues")
plt.pcolor(X, Y, Z2, alpha=0.9, cmap="Oranges")
plt.savefig("both_pcolor.pdf")
plt.clf()
plt.pcolor(X, Y, Z1 - Z2, cmap="plasma")
plt.savefig("diff_pcolor.pdf")
plt.clf()

G1 = makeGaussianPlot("Movie/Momentum/pdf_momentum_scan_pid211_step_092.root")
G2 = makeGaussianPlot("Movie/Momentum/pdf_momentum_scan_pid321_step_092.root")
plt.plot(G1, 'b-')
plt.plot(G2, 'r-')
plt.savefig("gaussians.pdf")
