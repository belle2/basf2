#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import progressbar
import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt

import ROOT
ROOT.gROOT.SetBatch(1)  # something's trying to open xwindows

from plotTOPPDFCollection import Gaussian, PDF

# file
f = ROOT.TFile.Open("TOPOutput.root")
t = f.Get("tree")
gcmap = 'plasma'

# plot integrated hitmap
bar = progressbar.ProgressBar(max_value=t.GetEntries())
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

# get array as return from histpgram function
(hits, _, _, _) = plt.hist2d(ch, times, bins=(512, 200), cmap=gcmap)
plt.savefig('debug_hitmap.pdf')
plt.clf()

# make the pdf from the first datum (FIXME: make this be truth momentum)
t.GetEntry(0)
x = t.TOPPDFCollections[0]
X, Y = np.meshgrid(np.linspace(0, 0.3 * 199, 200), range(512))
Z = np.zeros_like(X, dtype=np.float32)
for pixel, pxData in enumerate(x.m_data[13]):
    lOG = []  # list of Gaussians
    for peak, width, norm in pxData:
        lOG.append(Gaussian(peak, width, norm))
    Z[pixel, :] = np.array([PDF(lOG, timebin) for timebin in np.linspace(0, 0.3 * 199, 200)])

# as occupancy of hitmap may be zero, add machine level epsilon to take sqrt
eps = np.finfo(np.float32).eps
matrix_of_pulls = (hits - Z) / np.sqrt(hits + eps)
pulls = matrix_of_pulls.reshape(np.prod(matrix_of_pulls.shape))  # flattens to 1D list

plt.clf()
plt.hist(pulls, bins=np.linspace(-10, 10, 100))
plt.savefig("pulls_histogram.pdf")

plt.clf()
plt.pcolor(X, Y, matrix_of_pulls, cmap=gcmap)
plt.savefig("matrix_of_pulls.pdf")
f.Close()
