# !/usr/bin/env python3

""" plotting_utils.py : utility functions for plotting """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import os
import math
import ROOT


def get_ndf(data, fitres):
    """
    Return the number of degrees of freedom from RooDataHist and RooFitResult
    """
    nbins = data.numEntries()
    params = fitres.floatParsFinal().getSize()
    return nbins - params


def draw_plots(**kwargs):

    hypodir = "pdg{0}{1}".format(abs(kwargs["hypo"]), kwargs["append"])

    baseplotdir = "{0}/FitPlots/{1}".format(kwargs["outputplots"], hypodir)
    for plottype in ["LINY", "LOGY"]:
        plotdir = "{0}/{1}".format(baseplotdir, plottype)
        if not os.path.exists(plotdir):
            os.makedirs(plotdir)

    pdfplot = ROOT.TCanvas("pdfplot", "pdfplot", 1200, 1000)

    pad1 = ROOT.TPad("pad1", "", 0, 0.25, 1, 1)
    pad2 = ROOT.TPad("pad2", "", 0, 0, 1, 0.25)
    pad1.SetBottomMargin(0.02)
    pad2.SetBottomMargin(0.4)
    pad1.Draw()
    pad2.Draw()

    frame1 = kwargs["frame1"]

    pad1.cd()
    frame1.GetYaxis().SetTitleSize(0.05)
    frame1.GetYaxis().SetTitleOffset(0.8)
    frame1.GetXaxis().SetLabelSize(0)
    frame1.GetXaxis().SetLabelOffset(999)
    frame1.getAttText('pdf_paramBox').SetLineWidth(0)  # To remove fit results box border.
    frame1.getAttText('pdf_paramBox').SetFillStyle(0)  # To make the fit results box transparent.
    frame1.Draw()

    chi2 = frame1.chiSquare()

    t = ROOT.TLatex()
    t.SetNDC()
    t.SetTextColor(ROOT.kBlack)
    t.SetTextFont(43)
    t.SetTextSize(35)
    t.DrawLatex(0.15, 0.8, "#frac{{#chi^{{2}}}}{{ndf}} = #frac{{{0:.1f}}}{{{1}}}".format(chi2 * kwargs["ndf"], kwargs["ndf"]))

    frame2 = kwargs["frame2"]

    pad2.cd()
    frame2.GetYaxis().SetTitle(frame2.GetTitle())
    frame2.SetTitle("")
    frame2.GetYaxis().SetTitleSize(0.14)
    frame2.GetYaxis().SetTitleOffset(0.3)
    frame2.GetYaxis().SetLabelSize(0.1)
    frame2.GetXaxis().SetTitleSize(0.15)
    frame2.GetXaxis().SetLabelSize(0.15)
    frame2.GetXaxis().SetNdivisions(kwargs["ndiv"])
    frame2.Draw()

    badfit = "__BADFIT" if (chi2 > 500 or math.isnan(chi2)) else ""

    outname = "{0}/LINY/{1}_{2}_{3}{4}".format(baseplotdir,
                                               hypodir,
                                               kwargs["idx_p"],
                                               kwargs["idx_theta"],
                                               badfit)
    pdfplot.Print("{0}.pdf".format(outname))

    # Make log plot as well
    pad1.cd()
    pad1.SetLogy()
    outname = outname.replace("LINY", "LOGY")
    pdfplot.Print("{0}_LOGY.pdf".format(outname))

    pdfplot.Close()
