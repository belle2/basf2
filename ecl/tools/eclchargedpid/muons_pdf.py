# !/usr/bin/env python3

""" muons_pdf.py : construct the pdf for muon ECL charged PID. """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import ROOT
from plotting_utils import draw_plots, get_ndf

# WIP: fit showerE w/ Landau


def fit_muon_e(**kwargs):
    pass


def fit_muon_eop(**kwargs):

    append = "anti" if kwargs["charge"] > 0 else ""

    idx_p = kwargs["idx_p"]
    idx_th = kwargs["idx_theta"]

    # Set the PDF parameter ranges ((start), min, max).
    if kwargs["pmin"] < 5.5e3:
        eop_range = (0.0, 0.1)
        bifurgaus_mu_range = (0.05, 0.0, 0.1)
        bifurgaus_sigmaL_range = (0.003, 0.001, 0.005)
        bifurgaus_sigmaR_range = (0.1, 0.005, 0.12)
        bifurgaus_frac_range = (0.99, 1.0)
        gaus_mu_range = (0.075, 0.05, 0.1)
        gaus_sigma_range = (0.0, 0.02)
    if kwargs["pmin"] < 4.5e3:
        eop_range = (0.0, 0.1)
        bifurgaus_mu_range = (0.05, 0.035, 0.06)
        bifurgaus_sigmaL_range = (0.01, 0.005, 0.1)
        bifurgaus_sigmaR_range = (0.1, 0.005, 0.12)
        bifurgaus_frac_range = (0.99, 1.0)
        gaus_mu_range = (0.075, 0.05, 0.1)
        gaus_sigma_range = (0.0, 0.02)
    if kwargs["pmin"] < 4e3:
        eop_range = (0.0, 0.2)
        bifurgaus_mu_range = (0.06, 0.0, 0.15)
        bifurgaus_sigmaL_range = (0.055, 0.001, 0.01)
        bifurgaus_sigmaR_range = (0.028, 0.005, 0.05)
        bifurgaus_frac_range = (0.0, 1.0)
        gaus_mu_range = (0.1, 0.06, 0.2)
        gaus_sigma_range = (0.0, 0.04)
    if kwargs["pmin"] < 3e3:
        eop_range = (0.0, 0.2)
        bifurgaus_mu_range = (0.085, 0.0, 0.15)
        bifurgaus_sigmaL_range = (0.0125, 0.005, 0.03)
        bifurgaus_sigmaR_range = (0.0075, 0.005, 0.02)
        bifurgaus_frac_range = (0.0, 1.0)
        gaus_mu_range = (0.1, 0.06, 0.2)
        gaus_sigma_range = (0.0, 0.04)
    if kwargs["pmin"] < 2e3:
        eop_range = (0.0, 0.3)
        bifurgaus_mu_range = (0.16, 0.0, 0.3)
        bifurgaus_sigmaL_range = (0.019, 0.01, 0.1)
        bifurgaus_sigmaR_range = (0.033, 0.005, 0.1)
        bifurgaus_frac_range = (0.0, 1.0)
        gaus_mu_range = (0.16, 0.0, 0.3)
        gaus_sigma_range = (0.0, 0.1)
    if kwargs["pmin"] < 1e3:
        eop_range = (0.0, 0.6)
        bifurgaus_mu_range = (0.25, 0.0, 0.45)
        bifurgaus_sigmaL_range = (0.1, 0.01, 0.15)
        bifurgaus_sigmaR_range = (0.1, 0.005, 0.15)
        bifurgaus_frac_range = (0.0, 1.0)
        gaus_mu_range = (0.35, 0.2, 0.6)
        gaus_sigma_range = (0.0, 0.05)
    if kwargs["pmin"] < 0.5e3:
        eop_range = (0.0, 1.0)
        bifurgaus_mu_range = (0.27, 0.0, 0.5)
        bifurgaus_sigmaL_range = (0.145, 0.01, 0.3)
        bifurgaus_sigmaR_range = (0.1475, 0.005, 0.3)
        bifurgaus_frac_range = (0.0, 1.0)
        gaus_mu_range = (0.43, 0.1, 1.0)
        gaus_sigma_range = (0.0, 0.2)

    # Open file w/ input E/p distribution.
    inpath = "{0}/pdg{1}13.root".format(kwargs["inputpath"], append)
    infile = ROOT.TFile(inpath)

    # Create variable to fit and its distribution in data.
    eopvar = ROOT.RooRealVar("eop", "E/p", eop_range[0], eop_range[1])
    eophist_data = infile.Get("h_Eop_{0}_{1}".format(idx_p - 1, idx_th - 1))
    eopdata = ROOT.RooDataHist("eopdata", "eopdata", ROOT.RooArgList(eopvar), ROOT.RooFit.Import(eophist_data))

    # Create plot for the fit.
    pm = "+" if kwargs["charge"] > 0 else "-"
    particle = "#mu^{{{0}}}".format(pm)
    p_range = "{0:.2f} < p_{{lab}} #leq {1:.2f} [GeV/c]".format(kwargs["pmin"] / 1e3, kwargs["pmax"] / 1e3)
    theta_range = "{0:.1f} < #theta_{{lab}} #leq {1:.1f} [deg]".format(kwargs["thetamin"], kwargs["thetamax"])
    title = "{0}, {1}, {2}".format(particle, p_range, theta_range)
    frame1 = eopvar.frame(ROOT.RooFit.Title(title))
    eopdata.plotOn(frame1, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    # Create PDF to fit.
    bifurgaus_mu = ROOT.RooRealVar("#mu_{bg}",
                                   "mean of bifurcated gaussian",
                                   bifurgaus_mu_range[0],
                                   bifurgaus_mu_range[1],
                                   bifurgaus_mu_range[2])
    bifurgaus_sigmal = ROOT.RooRealVar("#sigma_{bg}_{L}",
                                       "widthL of bifurcated gaussian",
                                       bifurgaus_sigmaL_range[0],
                                       bifurgaus_sigmaL_range[1],
                                       bifurgaus_sigmaL_range[2])
    bifurgaus_sigmar = ROOT.RooRealVar("#sigma_{bg}_{R}",
                                       "widthR of bifurcated gaussian",
                                       bifurgaus_sigmaR_range[0],
                                       bifurgaus_sigmaR_range[1],
                                       bifurgaus_sigmaR_range[2])
    bifurgaus_frac = ROOT.RooRealVar("frac_{bg}",
                                     "bifurcated gaussian fraction",
                                     bifurgaus_frac_range[0],
                                     bifurgaus_frac_range[1])
    bifurgaus = ROOT.RooBifurGauss("bifurgaus", "bifurcated  gaussian", eopvar, bifurgaus_mu, bifurgaus_sigmal, bifurgaus_sigmar)

    gaus_mean = ROOT.RooRealVar("#mu_{g}",
                                "mean of gaussian",
                                gaus_mu_range[0],
                                gaus_mu_range[1],
                                gaus_mu_range[2])
    gaus_sigma = ROOT.RooRealVar("#sigma_{g}",
                                 "width of gaussian",
                                 gaus_sigma_range[0],
                                 gaus_sigma_range[1])
    gaus = ROOT.RooGaussian("gaus", "gaussian", eopvar, gaus_mean, gaus_sigma)

    pdf = ROOT.RooAddPdf("pdf", "bifurcated gaussian + gaussian", bifurgaus, gaus, bifurgaus_frac)

    # Ok, fit!
    fitres = pdf.fitTo(eopdata, ROOT.RooFit.Save(), ROOT.RooFit.PrintEvalErrors(-1))

    # Extract the normalised post-fit PDF as TF1.
    ral1 = ROOT.RooArgList(eopvar)
    ral2 = ROOT.RooArgList(pdf.getParameters(ROOT.RooArgSet(eopvar)))
    ras = ROOT.RooArgSet(eopvar)
    pdffunc = pdf.asTF(ral1, ral2, ras)

    # Add the PDFs and the fitted parameters to the plot.
    pdf.plotOn(frame1,
               ROOT.RooFit.LineColor(ROOT.kBlack),
               ROOT.RooFit.LineWidth(2),
               ROOT.RooFit.MoveToBack())
    pdf.plotOn(frame1,
               ROOT.RooFit.Components("bifurgaus"),
               ROOT.RooFit.LineStyle(ROOT.kDashed),
               ROOT.RooFit.LineColor(ROOT.kRed),
               ROOT.RooFit.LineWidth(2),
               ROOT.RooFit.MoveToBack())
    pdf.plotOn(frame1,
               ROOT.RooFit.Components("gaus"),
               ROOT.RooFit.LineStyle(ROOT.kSolid),
               ROOT.RooFit.LineColor(ROOT.kRed + 2),
               ROOT.RooFit.LineWidth(2),
               ROOT.RooFit.MoveToBack())
    pdf.paramOn(frame1, ROOT.RooFit.Layout(0.55, 0.9, 0.75))

    # Create plot for fit residuals.
    frame2 = eopvar.frame(ROOT.RooFit.Title("Residuals"))
    frame2.addPlotable(frame1.residHist(), "P")

    # Check the goodness of fit.
    ndf = get_ndf(eopdata, fitres)
    print("X^2/ndf = {0:.3f} (ndf = {1})".format(frame1.chiSquare(), ndf))

    # Draw the plots and save them.
    if kwargs["outputplots"]:
        plotargs = dict(kwargs, frame1=frame1.Clone(), frame2=frame2.Clone(), ndf=ndf, ndiv=510, append=append)
        draw_plots(**plotargs)

    pdfname = "pdf_EoP_{0}_{1}".format(idx_p, idx_th)
    return pdffunc.Clone(pdfname)  # Why cloning? A: https://root-forum.cern.ch/t/returning-th1f-from-function/17213/2
