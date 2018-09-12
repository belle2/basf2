# !/usr/bin/env python3

""" muons_pdf.py : construct the pdf for muon ECL charged PID. """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import os
import ROOT
from plotting_utils import draw_plots

# WIP: fit showerE w/ Landau


def fit_muon_e(**kwargs):
    pass


def fit_muon_eop(**kwargs):

    append = "anti" if kwargs["charge"] > 0 else ""

    eop_min = 0.0
    eop_max = 0.1 if kwargs["charge"] < 0 else 0.2

    idx_p = kwargs["idx_p"]
    idx_th = kwargs["idx_theta"]

    bifurgaus_mu_start = 0.05
    bifurgaus_mu_max = 0.2
    gaus_mu_max = 0.2
    gaus_sigma_max = 0.08

    if kwargs["pmin"] < 4e2:
        eop_max, bifurgaus_mu_start, bifurgaus_mu_max, gaus_mu_max, gaus_sigma_max = (1.0, 0.5, 0.8, 0.5, 0.35)
    elif kwargs["pmin"] < 5e2:
        eop_max, bifurgaus_mu_start, bifurgaus_mu_max, gaus_mu_max, gaus_sigma_max = (0.8, 0.2, 0.4, 0.3, 0.08)
    elif kwargs["pmin"] < 1e3:
        eop_max, bifurgaus_mu_start, bifurgaus_mu_max, gaus_mu_max, gaus_sigma_max = (0.6, 0.2, 0.4, 0.3, 0.08)
    elif kwargs["pmin"] < 4e3:
        eop_max, bifurgaus_mu_start, bifurgaus_mu_max, gaus_mu_max, gaus_sigma_max = (0.3, 0.2, 0.4, 0.3, 0.08)

    # Open file w/ input E/p distribution.
    inpath = "{0}/pdg{1}13.root".format(kwargs["inputpath"], append)
    infile = ROOT.TFile(inpath)

    # Create variable to fit and its distribution in data.
    eopvar = ROOT.RooRealVar("eop", "E/p (c)", eop_min, eop_max)
    eophist_data = infile.Get("h_Eop_{0}_{1}".format(idx_p - 1, idx_th - 1))
    eopdata = ROOT.RooDataHist("eopdata", "eopdata", ROOT.RooArgList(eopvar), ROOT.RooFit.Import(eophist_data))

    # Create plot for the fit.
    frame1 = eopvar.frame(ROOT.RooFit.Title("muons: E/p fit"))
    eopdata.plotOn(frame1, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    # Create PDF to fit.
    bifurgaus_mu = ROOT.RooRealVar("#mu_{bg}", "mean of bifurcated gaussian", bifurgaus_mu_start, 0.01, bifurgaus_mu_max)
    bifurgaus_sigmal = ROOT.RooRealVar("#sigma_{bg}_{L}", "widthL of bifurcated gaussian", 0.1, 0.01, 0.12)
    bifurgaus_sigmar = ROOT.RooRealVar("#sigma_{bg}_{R}", "widthR of bifurcated gaussian", 0.1, 0.005, 0.12)
    bifurgaus_frac = ROOT.RooRealVar("frac_{bg}", "bifurcated gaussian fraction", 0.00, 1.00)
    bifurgaus = ROOT.RooBifurGauss(
        "bifurgaus",
        "bifurcated  gaussian PDF",
        eopvar,
        bifurgaus_mu,
        bifurgaus_sigmal,
        bifurgaus_sigmar)

    gaus_mean = ROOT.RooRealVar("#mu_{g}", "mean of gaussian", 0.12, 0.01, gaus_mu_max)
    gaus_sigma = ROOT.RooRealVar("#sigma_{g}", "width of gaussian", 0.05, 0.005, gaus_sigma_max)
    gaus = ROOT.RooGaussian("gaus", "gaussian PDF", eopvar, gaus_mean, gaus_sigma)

    pdf = ROOT.RooAddPdf("pdf", "bifurcated gaussian + gaussian", bifurgaus, gaus, bifurgaus_frac)

    # Ok, fit!
    pdf.fitTo(eopdata)

    # Extract the normalised post-fit PDF as TF1.
    ral1 = ROOT.RooArgList(eopvar)
    ral2 = ROOT.RooArgList(pdf.getParameters(ROOT.RooArgSet(eopvar)))
    ras = ROOT.RooArgSet(eopvar)
    pdffunc = pdf.asTF(ral1, ral2, ras)

    # Add the PDFs and the fitted parameters to the plot.
    pdf.plotOn(frame1, ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineWidth(3), ROOT.RooFit.MoveToBack())
    bifurgaus.plotOn(
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

    # Check the goodness of fit.
    print("X^2/ndf = {0:.3f}".format(frame1.chiSquare()))

    # Create plot for fit residuals.
    frame2 = eopvar.frame(ROOT.RooFit.Title("Residual Distribution"))
    frame2.addPlotable(frame1.residHist(), "P")

    # Draw the plots and save them.
    if kwargs["outputplots"]:
        plotargs = dict(kwargs, frame1=frame1.Clone(), frame2=frame2.Clone(), append=append)
        draw_plots(**plotargs)

    pdfname = "pdf_EoP_{0}_{1}".format(idx_p, idx_th)
    return pdffunc.Clone(pdfname)  # Why cloning? A: https://root-forum.cern.ch/t/returning-th1f-from-function/17213/2
