# !/usr/bin/env python3

""" electrons_pdf.py : construct the pdf for electron ECL charged PID. """

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import ROOT
from plotting_utils import draw_plots, get_ndf


def fit_electron_eop(**kwargs):

    append = "anti" if kwargs["charge"] > 0 else ""

    idx_p = kwargs["idx_p"]
    idx_th = kwargs["idx_theta"]

    # Set the PDF parameter ranges ((start), min, max).
    eop_range = (0.0, 1.2)
    gaus_mu_range = (0.8, 0.5, 1.5)
    gaus_sigma_range = (0.01, 0.02, 0.1)
    gaus_frac_range = (0.0, 1.0)
    cb_mu_range = (1.0, 0.5, 1.5)
    cb_sigma_range = (0.1, 0.0001, 0.5)
    cb_alpha_range = (0.9, 0.2, 4.5)
    cb_nn_range = (1.35, 0.5, 10)
    if (kwargs["pmin"] < 5.5e3):
        gaus_sigma_range = (0.01, 0.005, 0.05)
    if (kwargs["pmin"] < 4e3 or kwargs["charge"] > 0):
        gaus_sigma_range = (0.01, 0.01, 0.05)
    if kwargs["pmin"] == 4.5e3:
        gaus_frac_range = (0.5, 1.0)  # Impose Gaussian fraction to be larger than CB fraction.
    if (kwargs["pmin"] < 0.5e3):
        gaus_frac_range = (0.0, 0.2)  # Impose Gaussian fraction to be smaller than CB fraction.

    # Open file w/ input E/p distribution.
    inpath = "{0}/pdg{1}11.root".format(kwargs["inputpath"], append)
    infile = ROOT.TFile(inpath)

    # Create variable to fit and its distribution in data.
    eopvar = ROOT.RooRealVar("eop", "E/p", eop_range[0], eop_range[1])
    eophist_data = infile.Get("h_Eop_{0}_{1}".format(idx_p - 1, idx_th - 1))
    eopdata = ROOT.RooDataHist("eopdata", "eopdata", ROOT.RooArgList(eopvar), ROOT.RooFit.Import(eophist_data))

    # Create plot for the fit.
    pm = "+" if kwargs["charge"] > 0 else "-"
    particle = "e^{{{0}}}".format(pm)
    p_range = "{0:.2f} < p_{{lab}} #leq {1:.2f} [GeV/c]".format(kwargs["pmin"] / 1e3, kwargs["pmax"] / 1e3)
    theta_range = "{0:.1f} < #theta_{{lab}} #leq {1:.1f} [deg]".format(kwargs["thetamin"], kwargs["thetamax"])
    title = "{0}, {1}, {2}".format(particle, p_range, theta_range)
    frame1 = eopvar.frame(ROOT.RooFit.Title(title))
    eopdata.plotOn(frame1, ROOT.RooFit.DataError(ROOT.RooAbsData.SumW2))

    # Create PDF to fit.
    gaus_mean = ROOT.RooRealVar("#mu_{g}",
                                "mean of gaussian",
                                gaus_mu_range[0],
                                gaus_mu_range[1],
                                gaus_mu_range[2])
    gaus_sigma = ROOT.RooRealVar("#sigma_{g}",
                                 "width of gaussian",
                                 gaus_sigma_range[0],
                                 gaus_sigma_range[1],
                                 gaus_sigma_range[2])
    gaus_frac = ROOT.RooRealVar("frac_{g}",
                                "gaussian fraction",
                                gaus_frac_range[0],
                                gaus_frac_range[1])
    gaus = ROOT.RooGaussian("gaus", "gaussian", eopvar, gaus_mean, gaus_sigma)

    cb_mu = ROOT.RooRealVar("#mu_{CB}",
                            "mean of CB",
                            cb_mu_range[0],
                            cb_mu_range[1],
                            cb_mu_range[2])
    cb_sigma = ROOT.RooRealVar("#sigma_{CB}",
                               "width of CB shape",
                               cb_sigma_range[0],
                               cb_sigma_range[1],
                               cb_sigma_range[2])
    cb_alpha = ROOT.RooRealVar("#alpha_{CB}",
                               "tail length",
                               cb_alpha_range[0],
                               cb_alpha_range[1],
                               cb_alpha_range[2])
    cb_nn = ROOT.RooRealVar("nn_{CB}",
                            "tail slope",
                            cb_nn_range[0],
                            cb_nn_range[1],
                            cb_nn_range[2])
    cb = ROOT.RooCBShape("cb", "crystal ball", eopvar, cb_mu, cb_sigma, cb_alpha, cb_nn)

    pdf = ROOT.RooAddPdf("pdf", "gaussian + crystal ball", gaus, cb, gaus_frac)

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
               ROOT.RooFit.Components("cb"),
               ROOT.RooFit.LineStyle(ROOT.kDashed),
               ROOT.RooFit.LineColor(ROOT.kTeal - 5),
               ROOT.RooFit.LineWidth(2),
               ROOT.RooFit.MoveToBack())
    pdf.plotOn(frame1,
               ROOT.RooFit.Components("gaus"),
               ROOT.RooFit.LineStyle(ROOT.kSolid),
               ROOT.RooFit.LineColor(ROOT.kGreen + 2),
               ROOT.RooFit.LineWidth(2),
               ROOT.RooFit.MoveToBack())
    pdf.paramOn(frame1, ROOT.RooFit.Layout(0.135, 0.45, 0.75))

    # Create plot for fit residuals.
    frame2 = eopvar.frame(ROOT.RooFit.Title("Residuals"))
    frame2.addPlotable(frame1.residHist(), "P")

    # Check the goodness of fit.
    ndf = get_ndf(eopdata, fitres)
    print("X^2/ndf = {0:.3f} (ndf = {1})".format(frame1.chiSquare(), ndf))

    # Draw the plots and save them.
    if kwargs["outputplots"]:
        plotargs = dict(kwargs, frame1=frame1.Clone(), frame2=frame2.Clone(), ndf=ndf, ndiv=520, append=append)
        draw_plots(**plotargs)

    pdfname = "pdf_EoP_{0}_{1}".format(idx_p, idx_th)
    return pdffunc.Clone(pdfname)  # Why cloning? A: https://root-forum.cern.ch/t/returning-th1f-from-function/17213/2
