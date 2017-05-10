#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import ROOT

f = ROOT.TFile.Open("TOPOutput_twotracks.root")
t = f.Get("tree")
gcmap = 'plasma'

# we create the image from the channels on X, and time bins on Y
X, Y = np.meshgrid(range(512), np.linspace(0, 0.3 * 199, 200))
zarrays = []
for i in range(t.GetEntries()):
    print('processing: ', i)
    # one entry per event
    t.GetEntry(i)
    # one collection per track
    x = t.TOPPDFCollections[0]
    # x.m_data is a map of hypotheses to a 2D PDF
    Z = np.array(x.m_data[1]).T
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
    plt.xlim(0, 512)
    plt.ylim(10, 59)
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
           norm=LogNorm(vmin=abs(diff.min()), vmax=abs(diff.max())))
ticks = np.logspace(np.log10(abs(diff.min())), np.log10(abs(diff.max())), num=5)
plt.colorbar(ticks=ticks)
print(ticks)
plt.xlim(0, 512)
plt.ylim(10, 59)
plt.xlabel('channel, $i_{ch}$')
plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
plt.savefig("pdf_diff0vs1_logz.pdf")
