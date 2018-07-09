# !/usr/bin/env python3

""" electrons_pdf.py : construct the pdf for electron ECL charged PID. """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import os
import ROOT
from plotting_utils import draw_plots


def fit_electron_eop(**kwargs):

    append = "anti" if kwargs["charge"] > 0 else ""

    eop_min = 0.0
    eop_max = 1.2

    idx_p = kwargs["idx_p"]
    idx_th = kwargs["idx_theta"]

    # Open file w/ input E/p distribution.
    inpath = "{0}/pdg{1}11.root".format(kwargs["inputpath"], append)
    infile = ROOT.TFile(inpath)

    # Create variable to fit and its distribution in data.
    eopvar = ROOT.RooRealVar("eop", "E/p (c)", eop_min, eop_max)
    eophist_data = infile.Get("h_Eop_{0}_{1}".format(idx_p - 1, idx_th - 1))
    eopdata = ROOT.RooDataHist("eopdata", "eopdata", ROOT.RooArgList(eopvar), ROOT.RooFit.Import(eophist_data))

    # Create plot for the fit.
    frame1 = eopvar.frame(ROOT.RooFit.Title("electrons: E/p fit"))
    eopdata.plotOn(frame1, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    # Create PDF to fit.
    gaus_sigma_min = 0.001
    gaus_sigma_max = 0.05 if (kwargs["pmin"] < 4e3 or kwargs["charge"] > 0) else 0.1

    gaus_mean = ROOT.RooRealVar("#mu_{1}", "mean of gaussian", 0.8, 0.5, 1.5)
    gaus_sigma = ROOT.RooRealVar("#sigma_{1}", "width of gaussian", 0.01, gaus_sigma_min, gaus_sigma_max)
    gaus_frac = ROOT.RooRealVar("frac_{g}", "gaussian fraction", 0.00, 1.00)
    gaus = ROOT.RooGaussian("gaus", "gaussian PDF", eopvar, gaus_mean, gaus_sigma)

    cb_mu = ROOT.RooRealVar("#mu_{CB}", "mean of CB", 1.0, 0.5, 1.5)
    cb_sigma = ROOT.RooRealVar("#sigma_{CB}", "width of CB shape", 0.1, 0.0001, 0.5)
    cb_alpha = ROOT.RooRealVar("#alpha_{CB}", "tail length", 0.9, 0.2, 4.5)
    cb_nn = ROOT.RooRealVar("nn_{CB}", "tail slope", 1.35, 0.5, 10)
    cb = ROOT.RooCBShape("CB", "crystal ball PDF", eopvar, cb_mu, cb_sigma, cb_alpha, cb_nn)

    pdf = ROOT.RooAddPdf("pdf", "gaussian + CB PDF", gaus, cb, gaus_frac)

    # Ok, fit!
    pdf.fitTo(eopdata)

    # Extract the normalised post-fit PDF as TF1.
    ral1 = ROOT.RooArgList(eopvar)
    ral2 = ROOT.RooArgList(pdf.getParameters(ROOT.RooArgSet(eopvar)))
    ras = ROOT.RooArgSet(eopvar)
    pdffunc = pdf.asTF(ral1, ral2, ras)

    # Add the PDFs and the fitted parameters to the plot.
    pdf.plotOn(frame1, ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineWidth(3), ROOT.RooFit.MoveToBack())
    cb.plotOn(
        frame1, ROOT.RooFit.LineStyle(
            ROOT.kDashed), ROOT.RooFit.LineColor(
            ROOT.TColor.GetColor(
                0, 180, 180)), ROOT.RooFit.LineWidth(5), ROOT.RooFit.MoveToBack())
    gaus.plotOn(
        frame1, ROOT.RooFit.LineStyle(
            ROOT.kDashed), ROOT.RooFit.LineColor(
            ROOT.TColor.GetColor(
                120, 60, 180)), ROOT.RooFit.LineWidth(5), ROOT.RooFit.MoveToBack())
    pdf.paramOn(frame1, ROOT.RooFit.Layout(0.65, 0.97, 0.94))

    # Create plot for fit residuals.
    frame2 = eopvar.frame(ROOT.RooFit.Title("Residual Distribution"))
    frame2.addPlotable(frame1.residHist(), "P")

    # Check the goodness of fit.
    print("X^2/ndf = {0:.3f}".format(frame1.chiSquare()))

    # Draw the plots and save them.
    if kwargs["outputplots"]:
        plotargs = dict(kwargs, frame1=frame1.Clone(), frame2=frame2.Clone(), append=append)
        draw_plots(**plotargs)

    pdfname = "pdf_EoP_{0}_{1}".format(idx_p, idx_th)
    return pdffunc.Clone(pdfname)  # Why cloning? A: https://root-forum.cern.ch/t/returning-th1f-from-function/17213/2
