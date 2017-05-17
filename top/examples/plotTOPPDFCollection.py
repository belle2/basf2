#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import ROOT

f = ROOT.TFile.Open("TOPOutput.root")
t = f.Get("tree")
gcmap = 'plasma'


def Gaussian(mean, width, norm):
    def g(x):
        return norm/width/np.sqrt(2*np.pi) * np.exp(-((mean - x)/width)**2)
    return g


def PDF(listOfGaussians, x):
    return sum(g(x) for g in listOfGaussians)


# we create the image from the channels (int64) on X, and time bins (float64) on Y
X, Y = np.meshgrid(range(512), np.linspace(10, 60, 150))
Z = np.zeros_like(X, dtype=np.float32)
zarrays = []
for i in range(t.GetEntries()):
    # one entry per event
    t.GetEntry(i)
    # one collection per track
    x = t.TOPPDFCollections[0]
    # x.m_data is a map of hypotheses to a 2D PDF
    for pixel, pxData in enumerate(x.m_data[13]):
        lOG = []  # list of Gaussians
        for peak, width, norm in pxData:
            lOG.append(Gaussian(peak, width, norm))
        Z[:, pixel] = np.array([PDF(lOG, x) for x in np.linspace(10, 60, 150)])
    zarrays += [Z]
    plt.pcolor(X, Y, Z, cmap=gcmap)

    ch = []
    times = []
    digits = t.TOPDigits
    for d in digits:
        ch.append(d.getPixelID())  # this is the same as channel in the pdf
        times.append(d.getTime())

    plt.plot(ch, times, 'ro', markersize=5)

    plt.colorbar()
    plt.xlim(-5, 515)
#    plt.ylim(10, 59)
    plt.xlabel('channel, $i_{ch}$')
    plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
    plt.savefig("pdf_%i.pdf" % i)
    plt.clf()

    # log z axis
    plt.pcolor(X, Y, Z, cmap=gcmap, norm=LogNorm())
    plt.plot(ch, times, 'ro', markersize=5)
    plt.colorbar()
    plt.xlim(0, 512)
    plt.ylim(10, 59)
    plt.xlabel('channel, $i_{ch}$')
    plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
    plt.savefig("pdf_%i_logz.pdf" % i)
    plt.clf()

diff = zarrays[0] - zarrays[1]
print(np.sum(diff), np.sum(np.abs(diff)))
plt.pcolor(X, Y, diff, cmap=gcmap)
plt.colorbar()
plt.xlabel('channel, $i_{ch}$')
plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
plt.xlim(0, 512)
plt.ylim(10, 59)
plt.savefig("pdf_diff0vs1.pdf")

# log z axis
plt.clf()
plt.pcolor(X, Y, abs(diff), cmap=gcmap,
           norm=LogNorm())
# ticks = np.logspace(np.log10(abs(diff.min())), np.log10(abs(diff.max())), num=5)
plt.colorbar()
plt.xlim(0, 512)
plt.ylim(10, 59)
plt.xlabel('channel, $i_{ch}$')
plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
plt.savefig("pdf_diff0vs1_logz.pdf")
