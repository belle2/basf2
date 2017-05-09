#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import ROOT
import progressbar

f = ROOT.TFile.Open("TOPOutput.root")
t = f.Get("tree")
gcmap = 'plasma'

# we create the image from the channels on X, and time bins on Y
bar = progressbar.ProgressBar(max_value=t.GetEntries())
X, Y = np.meshgrid(range(512), np.linspace(0, 0.3 * 199, 200))
zarrays = []
ch = []
times = []
for i in range(t.GetEntries()):
    if i % 100 == 0:
        bar.update(i)
    t.GetEntry(i)

    digits = t.TOPDigits
    for d in digits:
        ch.append(d.getPixelID())  # actually the same as pdf channel
        times.append(d.getTime())

plt.hist2d(ch, times, bins=1000, cmap=gcmap)
plt.colorbar()
plt.xlim(0, 512)
plt.ylim(0, 75)
plt.xlabel('channel, $i_{ch}$')
plt.ylabel('time of propagation, $t_{TOP}$ (TDC)')
plt.savefig("hitmap.pdf")
plt.clf()

plt.clf()
plt.hist2d(ch, times, bins=1000, cmap=gcmap, norm=LogNorm())
plt.colorbar()
plt.xlim(0, 512)
plt.ylim(0, 75)
plt.xlabel('channel, $i_{ch}$')
plt.ylabel('time of propagation, $t_{TOP}$ (TDC)')
plt.savefig("hitmap_logy.pdf")
f.Close()
