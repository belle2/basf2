#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------------------------
# Plot TOP probability distribution from TOPOutput.root
# This script assumes you've already run:
#   basf2 getTOPPdf.py -n 2 # or larger
# or a similar script that writes out the TOP probability distributions. Not
# done in default TOPreco setup
# -----------------------------------------------------------------------------

__authors__ = ['Sam Cunliffe', 'Jan Strube']

import argparse
import numpy as np


def arguments():
    """Parse command line options to this script"""
    ap = argparse.ArgumentParser()
    ap.add_argument("-i", "--input", help="Input TOPOutput file", default="TOPOutput.root")
    ap.add_argument("-t", "--tree", help="TTree name", default="tree")
    ap.add_argument("-c", "--colour", help="Matplotlib colourmap", default="plasma")
    ap.add_argument("-x", "--xaxis", help="Option for x axis arrangement", default="pixel")
    return ap.parse_args()


def Gaussian(mean, width, norm):
    """Make a Gaussian function"""
    def g(x):
        return norm / width / np.sqrt(2 * np.pi) * np.exp(-((mean - x) / width)**2)
    return g


def PDF(listOfGaussians, x):
    """Remake the pdf for a single pixel (a sum of many Gaussians)"""
    return sum(g(x) for g in listOfGaussians)


def plot_single_event_hitmap(digits):
    """Plot the hitmap for a single event"""
    ch = []
    times = []
    for d in digits:
        # only run over good hits. 1 == c_Good
        if d.getHitQuality() != 1:
            continue
        ch.append(d.getPixelID())  # this is the same as channel in the pdf
        times.append(d.getTime())

    plt.plot(ch, times, 'ro', markersize=5)
    plt.colorbar()
    plt.xlim(-5, 515)
    plt.xlabel('pixel number')
    plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
    plt.savefig("pdf_%i.pdf" % i)
    plt.clf()
    plt.plot(ch, times, 'ro', markersize=5)
    return


if __name__ == "__main__":
    # all the graphical imports are only necessary
    # if this script gets run, not if it's only imported

    # looks redundant, but it's needed for projection='3d'
    from mpl_toolkits.mplot3d import Axes3D
    import matplotlib
    matplotlib.use('pdf')
    import matplotlib.pyplot as plt
    from matplotlib.colors import LogNorm
    import ROOT
    import sys

    # get input
    args = arguments()
    f = ROOT.TFile.Open(args.input)
    t = f.Get(args.tree)

    # if args.xaxis in ["pixel", "pixelID"]:
    xbins = np.linspace(0, 512, 513)
    # ybins = np.linspace(0, 80, 201)
    ybins = np.linspace(20, 60, 201)
    xcentres = xbins[0:-1] + 0.5 * (xbins[1:] - xbins[:-1])
    ycentres = ybins[0:-1] + 0.5 * (ybins[1:] - ybins[:-1])

    # create the image from the channels (int64) on X, and time bins (float64) on Y
    X, Y = np.meshgrid(xcentres, ycentres)
    zarrays = []

    # one track per event (particle gun)
    for iEv in range(t.GetEntries()):
        t.GetEntry(iEv)

        # one collection per track
        x = t.TOPPDFCollections[0]

        Z = np.zeros_like(X, dtype=np.float32)
        # x.m_data is a map of hypotheses to a 2D PDF which is a sum of Gaussians
        for pixel, pxData in enumerate(x.m_data[13]):
            lOG = []  # list of Gaussians
            for peak, width, norm in pxData:
                lOG.append(Gaussian(peak, width, norm))
            Z[:, pixel] = np.array([PDF(lOG, x) for x in ycentres])

        plt.plot(ycentres, Z[:, 500])
        plt.savefig('single.pdf')
        plt.clf()

        # optional reordering of pixels first y then x
        if args.xaxis in ['reordered']:
            # rearrange the x centres list
            newIDX = np.ravel(np.column_stack(xcentres.reshape(8, 64)))
            # make it bin edges
            newIDX = np.array(newIDX - 0.5, dtype=np.int64)
            # re-order the Z
            Z = Z[:, newIDX]

        zarrays.append(Z)

        # factored all of this to a function for reintegration later
        # digits = t.TOPDigits
        # plot_single_event_hitmap(digits)

        if args.xaxis in ['reordered']:
            plt.pcolor(X, Y, Z, cmap=args.colour)
            plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
            # change label and hide ticks (since in this case it's meaninglesS)
            frame = plt.gca()
            frame.axes.get_xaxis().set_ticks([])  # empty ticks
            plt.xlabel('reordered pixel number')  # but still title
            plt.savefig("pdf_%i_reordered.pdf" % iEv)
            # log z axis
            plt.clf()
            plt.pcolor(X, Y, Z, cmap=args.colour, norm=LogNorm())
            plt.xlim(-5, 515)
            plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
            frame = plt.gca()
            frame.axes.get_xaxis().set_ticks([])  # empty ticks
            plt.xlabel('reordered pixel number')  # but still title
            plt.savefig("pdf_%i_logy_reordered.pdf" % iEv)
        elif args.xaxis in ["layered"]:
            for irow in range(7, -1, -1):
                X, Y = np.meshgrid(xcentres[:64], ycentres)
                plt.pcolor(X, Y, Z[:, irow * 64:(irow + 1) * 64], cmap=args.colour, alpha=0.2)
                plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
                plt.xlabel('PMT column number')
            plt.savefig("pdf_layers_%i.pdf" % iEv)
        elif args.xaxis in ["layered3D", "layered3d"]:
            fig = plt.figure()
            ax = fig.gca(projection="3d")
            X, Y = np.meshgrid(xcentres[:64], ycentres)
            for irow in range(7, -1, -1):
                zz = irow * np.ones_like(X)
                Zstrip = Z[:, irow * 64:(irow + 1) * 64]
                Zscstr = Zstrip / np.max(Zstrip)  # - np.ones_like(Zstrip)*0.1
                # Zscstr = np.zeros_like(Zstrip)
                # fc = plt.cm.YlGnBu(Zscstr, alpha=0.2)
                fc = plt.cm.binary(Zscstr, alpha=0.2)

                # fc = plt.cm.ScalarMappable(Zscstr, cmap=args.colour)
                # fc = plt.cm.Greys(np.log(Z[:, irow*64:(irow+1)*64]), alpha=0.4)
                ax.plot_surface(X, Y, zz, facecolors=fc, linewidth=0.)
                plt.gca().patch.set_facecolor('white')
                # make grid backgrounds transparent
                ax.xaxis.set_pane_color((1.0, 1.0, 1.0, 0.0))
                ax.yaxis.set_pane_color((1.0, 1.0, 1.0, 0.0))
                ax.zaxis.set_pane_color((1.0, 1.0, 1.0, 0.0))
                # make the grid lines transparent
                ax.xaxis._axinfo["grid"]['color'] = (1, 1, 1, 0)
                ax.yaxis._axinfo["grid"]['color'] = (1, 1, 1, 0)
                ax.zaxis._axinfo["grid"]['color'] = (1, 1, 1, 0)
                # axes labels
                ax.set_zlabel("PMT array row number")
                ax.set_ylabel('time of propagation, $t_{TOP}$ (ns)')
                ax.set_xlabel('PMT array column number')
            plt.savefig("pdf_layers_%i.pdf" % iEv)
            plt.savefig("pdf_layers_%i.png" % iEv)
        else:
            plt.pcolor(X, Y, Z, cmap=args.colour)
            plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
            plt.xlabel('pixel number')
            plt.savefig("pdf_%i.pdf" % iEv)
            # log z axis
            plt.clf()
            plt.pcolor(X, Y, Z, cmap=args.colour, norm=LogNorm())
            plt.xlim(-5, 515)
            plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
            plt.xlabel('pixel number')
            plt.savefig("pdf_%i_logy.pdf" % iEv)

        plt.clf()

    if len(zarrays) > 1:
        # then we made more rthan two plots so make a diff of the first two
        diff = zarrays[0] - zarrays[1]
        plt.pcolor(X, Y, diff, cmap=args.colour)
        plt.colorbar()
        plt.xlabel('channel, $i_{ch}$')
        plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
        plt.xlim(0, 512)
        plt.ylim(10, 59)
        plt.savefig("pdf_diff0vs1.pdf")

        # log z axis
        plt.clf()
        plt.pcolor(X, Y, abs(diff), cmap=args.colour, norm=LogNorm())
        plt.colorbar()
        plt.xlim(0, 512)
        plt.ylim(10, 59)
        plt.xlabel('channel, $i_{ch}$')
        plt.ylabel('time of propagation, $t_{TOP}$ (ns)')
        plt.savefig("pdf_diff0vs1_logz.pdf")
