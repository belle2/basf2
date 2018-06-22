# !/usr/bin/env python3

""" plotting_utils.py : utility functions for plotting """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import os
import ROOT


def draw_plots(**kwargs):

    plotdir = "{0}/FitPlots".format(kwargs["outputpath"])
    if not os.path.exists(plotdir):
        os.makedirs(plotdir)

    pdfplot = ROOT.TCanvas("pdfplot", "pdfplot", 1200, 1000)
    pdfplot.Divide(1, 2)

    frame1 = kwargs["frame1"]

    pdfplot.cd(1)
    frame1.GetYaxis().SetTitleOffset(0.7)
    frame1.GetXaxis().SetTitleOffset(0.7)
    frame1.GetXaxis().SetLabelSize(0.04)
    frame1.GetYaxis().SetLabelSize(0.04)
    frame1.GetXaxis().SetTitleSize(0.06)
    frame1.GetYaxis().SetTitleSize(0.06)
    frame1.Draw()
    t = ROOT.TText(0.5, 0.1, " ")
    t.SetTextColor(ROOT.kBlack)
    t.SetTextFont(43)
    t.SetTextSize(26)
    t.DrawTextNDC(0.13, 0.86, "X^2/ndf = {0:.3f}".format(frame1.chiSquare()))
    t.Draw()

    frame2 = kwargs["frame2"]

    pdfplot.cd(2)
    frame2.GetYaxis().SetTitleOffset(0.7)
    frame2.GetXaxis().SetTitleOffset(0.7)
    frame2.GetXaxis().SetLabelSize(0.04)
    frame2.GetYaxis().SetLabelSize(0.04)
    frame2.GetXaxis().SetTitleSize(0.06)
    frame2.GetYaxis().SetTitleSize(0.06)
    frame2.Draw()

    pdfplot.Print("{0}/pdg{1}{2}_{3}_{4}.pdf".format(plotdir,
                                                     kwargs["hypo"], kwargs["append"], kwargs["idx_p"], kwargs["idx_theta"]))
    pdfplot.Close()
