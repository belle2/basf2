#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
# Given the reconstructed data from runSimReco_Belle3_validation
# makes resolution plots (d0, z0, pt)
#
# ./plot_resolution.py -f1 belle3_analysis_dimuon_all_charged.root
#
"""

import ROOT as r
import numpy as np
from array import array
import scipy
import os
import shutil


def IPandFit(tree, variable, interval, cut, method, save_method=False):

    intervalGraph = []
    intervalError = []

    for i in range(len(interval) - 1):
        intervalGraph.append((interval[i] + interval[i + 1]) / 2)
        intervalError.append(interval[i + 1] - intervalGraph[i])

    resolution = []
    resolution_Error = []

    if cut != "":
        var = "_" + variable + "_" + method + "_" + cut
        cut = " && " + cut

    else:
        var = "_" + variable + "_" + method

    if variable == "d0":
        draw1 = "(d0)*10000>>thRes" + var
        range1 = "(p*(p/E)*(sin(theta))^(3/2))>="
        range2 = " && (p*(p/E)*(sin(theta))^(3/2))<"

        xminFitGaus = -400
        xmaxFitGaus = 400

    if variable == "z0":
        draw1 = "(z0)*10000>>thRes" + var
        range1 = "(p*(p/E)*(sin(theta))^(5/2))>="
        range2 = " && (p*(p/E)*(sin(theta))^(5/2))<"

        xminFitGaus = -400
        xmaxFitGaus = 400

    if variable == "pt":
        draw1 = "(pt-mcPT)/mcPT>>thRes" + var
        range1 = "mcPT>="
        range2 = " && mcPT<"

        xminFitGaus = -0.03
        xmaxFitGaus = 0.03

    if method == "fit":
        thRes = r.TH1D('thRes' + var, var + ' residus', 20, xminFitGaus, xmaxFitGaus)
        gauss = r.TF1("gauss" + var, "gaus", xminFitGaus, xmaxFitGaus)

    if method == "quantiles":
        thRes = r.TH1D('thRes' + var, var + ' residus', 100, xminFitGaus, xmaxFitGaus)
        thResTemp = r.TH1D('thResTemp' + var, var + ' residus', 100, xminFitGaus, xmaxFitGaus)

        nq = 3
        xq = array('d', [0.] * nq)  # position where to compute the quantiles in [0,1]
        yq = array('d', [0.] * nq)  # array to contain the quantiles

        xq[0] = 0.16  # 16th pct
        xq[1] = 0.5
        xq[2] = 0.84  # 84th pct

    # cutPrimary = " && mcPrimary==1 && mcInitial==0 && mcVirtual==0 && isSignal==1"
    cutPrimary = " && mcPrimary==1 && mcInitial==0 && mcVirtual==0"

    gRandom = r.TRandom3(0)

    for i in range(0, len(interval) - 1):

        tree.Draw(draw1, range1 + str(interval[i]) + range2 + str(interval[i + 1]) + cutPrimary + cut, "goff")

        q16 = []
        q84 = []

        if method == "fit":
            thRes.Fit(gauss, 'NQ')

            resolution.append(gauss.GetParameter(2))
            resolution_Error.append(gauss.GetParError(2))

        if method == "rms":
            resolution.append(thRes.GetRMS())
            resolution_Error.append(thRes.GetRMSError())

        if method == "quantiles":

            # Bootstrapping to get the 16th & 84th pct values, and their uncertainties
            for j in range(1000):
                thResTemp.Reset()
                thResTemp.FillRandom(thRes, int(thRes.GetEntries()))
                thResTemp.GetQuantiles(nq, yq, xq)
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
                s16 += (q16[j] - x16) * (q16[j] - x16)
                s84 += (q84[j] - x84) * (q84[j] - x84)

            s16 /= (len(q16) - 1)
            s84 /= (len(q84) - 1)

            s16 = np.sqrt(s16)
            s84 = np.sqrt(s84)

            # sigma(d0) = (x84-x16)/2
            resolution.append((x84 - x16) / 2)
            resolution_Error.append((s16 + s84) / 2)

        # to see if the method performs well
        if save_method:
            c1 = r.TCanvas('c1', 'Results', 900, 900)

            path = "fit/"

            if not os.path.isdir(path):
                os.mkdir(path)

            if not os.path.isdir(path + var[1:]):
                os.mkdir(path + var[1:])

            thRes.Draw()
            if method == "fit":
                gauss.Draw("SAME")
            if method == "quantiles":
                line0 = r.TLine(yq[0], 0, yq[0], 10000)
                line0.Draw("SAME")
                line1 = r.TLine(yq[1], 0, yq[1], 10000)
                line1.Draw("SAME")
                line2 = r.TLine(yq[2], 0, yq[2], 10000)
                line2.Draw("SAME")
            r.gPad.Update()
            c1.SaveAs(path + var[1:] + "/" + str(interval[i]) + "_" + str(interval[i + 1]) + ".pdf")

            c1.cd(1).Clear()
            c1.Close()

    gResolution = r.TGraphErrors(len(intervalGraph), scipy.array(intervalGraph), scipy.array(resolution),
                                 scipy.array(intervalError), scipy.array(resolution_Error))

    if variable != "pt":

        # used to fit with option I (not available for TGraphErrors)
        gResolution = r.TH1D("gResolution" + var, "ThFit", len(interval) - 1, scipy.array(interval))

        for i in range(len(resolution)):
            gResolution.SetBinContent(i + 1, resolution[i])
            gResolution.SetBinError(i + 1, resolution_Error[i])

        xminRes = interval[1]
        xmaxRes = interval[-1]

        resFit = r.TF1("resFit" + var, "sqrt([0]*[0]+([1]/x)*([1]/x))", xminRes, xmaxRes)

        resFit.SetParNames("a" + var, "b" + var)

        resFit.SetParameters(10., 20.)

        # gResolution.Fit(resFit,"0R+")
        gResolution.Fit(resFit, "I0R+")

        return gResolution, resFit

    else:
        return gResolution


def plot_IP(root, interval, out_file, save_method=False):

    f1 = r.TFile.Open(root)

    t1 = f1.variables

    gResolution_D0, resFit_D0 = IPandFit(t1, "d0", interval, "", "quantiles", save_method)
    gResolution_Z0, resFit_Z0 = IPandFit(t1, "z0", interval, "", "quantiles", save_method)
    gResolution_PT = IPandFit(t1, "pt", interval, "", "quantiles", save_method)

    c2 = r.TCanvas('c2', 'Results', 900, 900)
    c2.Divide(2, 2)

    r.gStyle.SetOptStat(0)

    c2.cd(1)
    r.gPad.SetLeftMargin(0.15)
    resFit_D0.SetTitle("resolution_D0")
    resFit_D0.SetMaximum(140)
    resFit_D0.SetMinimum(0)
    resFit_D0.GetYaxis().SetTitle("\\sigma(d0)")
    resFit_D0.GetXaxis().SetTitle(r"p\beta sin^{3/2}(\theta)")
    resFit_D0.Draw("")
    gResolution_D0.Draw("SAMES")

    c2.cd(2)
    r.gPad.SetLeftMargin(0.15)
    resFit_Z0.SetTitle("resolution_Z0")
    # resFit_Z0.SetMaximum(140)
    resFit_Z0.SetMinimum(0)
    resFit_Z0.GetYaxis().SetTitle("\\sigma(z0)")
    resFit_Z0.GetXaxis().SetTitle(r"p\beta sin^{5/2}(\theta)")
    resFit_Z0.Draw("")
    gResolution_Z0.Draw("SAMES")

    c2.cd(3)
    r.gPad.SetLeftMargin(0.15)
    r.gPad.SetLogy()
    gResolution_PT.SetTitle("resolution_PT")
    gResolution_PT.SetMaximum(1.01)
    gResolution_PT.SetMinimum(0.001)
    gResolution_PT.GetXaxis().SetTitle("pt_{MC}")
    gResolution_PT.GetYaxis().SetTitle("(pt_{gen}-pt_{MC})/pt_{MC}")
    gResolution_PT.Draw("AP")

    c2.cd(4).Clear()

    c2.Print(out_file)


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-f1', '--file1', required=True, type=str, help='Path to the reconstructed data')
    args = parser.parse_args()

    r.gStyle.SetOptStat(2222)
    r.gStyle.SetOptFit(1111)
    r.gStyle.SetStatW(0.3)
    r.gStyle.SetStatH(0.1)

    c = r.TCanvas('c', 'Results', 900, 900)
    out_file = 'resolutions.pdf'
    c.Print(out_file + '[')

    # Rootfiles used
    rootfiles = args.file1

    # Resolutions
    interval = [0., 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.75, 1., 1.5, 2., 2.5, 3., 3.5, 4., 5.]
    # interval = [0., 0.05, 0.1, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5]

    plot_IP(rootfiles, interval, out_file)

    c.Print(out_file + ']')
    c.Close()
    input()
