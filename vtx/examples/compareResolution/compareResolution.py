##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import ROOT
import matplotlib.pyplot as plt
import numpy as np
from array import array
import os

gRandom = ROOT.TRandom3(0)

p_s = "pt*sqrt(1+tanlambda*tanlambda)*sqrt(sin(3.14/2.-atan(tanlambda))*sin(3.14/2.-atan(tanlambda))*sin(3.14/2.-atan(tanlambda)))"
d0_s = "d0*10000"


def d0ResolutionPlot(
        file1,
        file2=None,
        name1='file1',
        name2='file2',
        n_bins=15,
        max_d0=200,
        max_z0=0.4,
        z0_shifts=None,
        min_pxd=0,
        title=""):
    if z0_shifts is None:
        z0_shifts = [0., 0.]
    f = ROOT.TFile(file1)
    c = ROOT.TCanvas()

    fig = plt.figure()
    ax1 = fig.add_subplot(111)

    def add_data(color='red', label="MC dimuons", method='quantiles', save_fit_step=False):
        d0pt = ROOT.TH2F("d0pt", "corrected d0 vs. pt", n_bins, 0.1, 5.0, 200, -max_d0, max_d0)

        f.Get("variables").Draw(
            d0_s + ':' + p_s + '>> d0pt',
            "abs(d0*10000) < {} && abs(z0 - {}) < {} && nPXDHits>={}".format(
                max_d0,
                z0_shift,
                max_z0,
                min_pxd),
            "goff")

        x = []
        y = []
        ey = []
        ex = []

        for ibin in range(1, d0pt.GetNbinsX() + 1):
            proj = d0pt.ProjectionY("p", ibin, ibin + 1)
            x.append(d0pt.GetXaxis().GetBinCenter(ibin))
            ex.append(d0pt.GetXaxis().GetBinWidth(ibin) / 2.)

            if method == 'fit':
                fcn = ROOT.TF1("fit", "gaus")
                proj.Fit("fit", "q")
                val = proj.GetFunction("fit").GetParameter(2)
                valerr = proj.GetFunction("fit").GetParError(2)
                y.append(val)
                ey.append(proj.GetRMSError())

            if method == 'quantiles':
                proj_temp = d0pt.ProjectionY("p2", ibin, ibin + 1)

                nq = 3
                xq = array('d', [0.] * nq)  # position where to compute the quantiles in [0,1]
                yq = array('d', [0.] * nq)  # array to contain the quantiles

                xq[0] = 0.16  # 16th pct
                xq[1] = 0.5  # median, not used
                xq[2] = 0.84  # 84th pct

                q16 = []
                q84 = []

                # Bootstrapping to get the 16th & 84th pct values, and their uncertainties
                for j in range(1000):
                    proj_temp.Reset()
                    proj_temp.FillRandom(proj, int(proj.GetEntries()))
                    proj_temp.GetQuantiles(nq, yq, xq)
                    q16.append(yq[0])
                    q84.append(yq[2])

                # Average for the 16th & 84th pct
                x16, x84 = 0, 0

                for j in range(len(q16)):
                    x16 += q16[j]
                    x84 += q84[j]

                x16 /= len(q16)
                x84 /= len(q84)

                # 16th & 84th pct uncertainties
                s16, s84 = 0, 0

                for j in range(len(q16)):
                    s16 += (q16[j] - x16)*(q16[j] - x16)
                    s84 += (q84[j] - x84)*(q84[j] - x84)

                s16 /= (len(q16)-1)
                s84 /= (len(q84)-1)

                s16 = np.sqrt(s16)
                s84 = np.sqrt(s84)

                # sigma(d0) = (x84-x16)/2
                y.append((x84-x16)/2)
                ey.append((s16+s84)/2)

                if save_fit_step:
                    c1 = ROOT.TCanvas('c1', 'Results', 900, 900)

                    path = "fit/"

                    if not os.path.isdir(path):
                        os.mkdir(path)

                    proj.Draw()
                    line0 = ROOT.TLine(yq[0], 0, yq[0], 10000)
                    line0.Draw("SAME")
                    line1 = ROOT.TLine(yq[1], 0, yq[1], 10000)
                    line1.Draw("SAME")
                    line2 = ROOT.TLine(yq[2], 0, yq[2], 10000)
                    line2.Draw("SAME")
                    ROOT.gPad.Update()
                    c1.SaveAs(path+"/"+str(ibin)+".pdf")

                    c1.cd(1).Clear()
                    c1.Close()

        ax1.errorbar(x, y, yerr=ey, xerr=ex, fmt='o', color=color, label=label)

    z0_shift = z0_shifts[0]
    add_data(label=name1, color='red')
    if file2 is not None:
        f = ROOT.TFile(file2)
        z0_shift = z0_shifts[1]
        add_data(color='blue', label=name2)
    plt.title(title)
    plt.xlabel("pseudo-momentum [GeV/c]")
    plt.ylabel("sigma(d0) [um]")
    import matplotlib.patches as mpatches
    red_patch = mpatches.Patch(color='red', label=name1)
    blue_patch = mpatches.Patch(color='blue', label=name2)
    plt.grid()
    plt.legend(handles=[red_patch, blue_patch])
    fig.show()


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("-f1", "--file1", required=True, help="Path to first root file")
    ap.add_argument("-f2", "--file2", required=True, help="Path to second root file")
    ap.add_argument("-n1", "--name1", required=True, help="Name of first detector")
    ap.add_argument("-n2", "--name2", required=True, help="Name of second detector")
    args = vars(ap.parse_args())

    plot1 = d0ResolutionPlot(args["file1"],  args["file2"], args["name1"],  args["name2"])
    # plot1 = d0ResolutionPlot(args["file1"])
    input()
