#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import progressbar as pb
import argparse as ap
import numpy as np
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt

import ROOT
ROOT.gROOT.SetBatch(1)  # something's trying to open xwindows

from plotTOPPDFCollection import Gaussian, PDF

# command line arguments
parser = ap.ArgumentParser('plot pulls from the pdf and many topdigits')
parser.add_argument('--input', '-i', default='TOPOutput.root',
                    help='file containing toppdfcollection and topdigits')
parser.add_argument('--debug', help='run in debug', action='store_true')
args = parser.parse_args()

# file
f = ROOT.TFile.Open(args.input)
t = f.Get("tree")
gcmap = 'plasma'

# plot integrated hitmap
bar = pb.ProgressBar(max_value=t.GetEntries())
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
(hits, xs, ys, _) = plt.hist2d(ch, times, bins=(512, 200), cmap=gcmap)

# for some reason this needs to be transposed to be compatible with meshgrids
hits = hits.T
xcentres = xs[1:] - 0.5 * (xs[1] - xs[0])
ycentres = ys[1:] - 0.5 * (ys[1] - ys[0])

# make the pdf from the first datum (FIXME: make this be truth momentum)
t.GetEntry(0)
x = t.TOPPDFCollections[0]
X, Y = np.meshgrid(xcentres, ycentres)
Z = np.zeros_like(X, dtype=np.float32)
for pixel, pxData in enumerate(x.m_data[13]):
    lOG = []  # list of Gaussians
    for peak, width, norm in pxData:
        lOG.append(Gaussian(peak, width, norm))
        Z[:, pixel] = np.array([PDF(lOG, timebin) for timebin in ycentres])

# normalise hitmap and scale it to the pdf integral
if args.debug:
    print(np.sum(Z))
    print(np.sum(hits))

hits = hits * np.sum(Z) / np.sum(hits)

if args.debug:
    print(np.sum(Z))
    print(np.sum(hits))

    plt.colorbar()
    plt.savefig('debug_hitmap.pdf')
    plt.clf()

    plt.pcolor(X, Y, Z, cmap=gcmap)
    plt.colorbar()
    plt.savefig('debug_pdf.pdf')
    plt.clf()


# as occupancy of hitmap may be zero, add machine level epsilon to take sqrt
eps = np.finfo(np.float32).eps
matrix_of_pulls = (hits - Z) / np.sqrt(hits + eps)
pulls = matrix_of_pulls.reshape(np.prod(matrix_of_pulls.shape))  # flattens to 1D list

# plot pulls
plt.clf()
plt.hist(pulls, bins=np.linspace(-5, 5, 100))
plt.savefig("pulls_histogram.pdf")

# plot the pulls as a finction of time and channel... i.e. a pull 'hitmap'
plt.clf()
plt.pcolor(X, Y, matrix_of_pulls, cmap=gcmap)
plt.colorbar()
plt.savefig("matrix_of_pulls.pdf")
f.Close()
