#!/usr/bin/env python3

"""
<header>
<output>validationTestPlots.root, validationTestPlotsExpertOnly.root</output>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>

<description>
This file will generate various output into a root-file to have
a fast turn-around during development
</description>
</header>
"""


import basf2
import array
import numpy as np
import ROOT
from datetime import datetime
from validationtest import add_properties


if __name__ == "__main__":
    # make sure we are able to always create the same plots
    basf2.set_random_seed(1337)

    tfile = ROOT.TFile("validationTestPlots.root", "RECREATE")

    # NTuples
    # ======================================

    tntuple = ROOT.TNtuple("ntuple_test", "ntuple test", "x:y:z:k")

    array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0])
    tntuple.Fill(array_of_values)

    tntuple.SetAlias('Description', r"This is a description test. "
                     r"Lorem ipsum sit dolor amet.  We also support $\LaTeX$! "
                     "\n <br> \n For example, here is the infamous "
                     "Einstein-Pythagoras-theorem: \n "
                     r"$$a^2 + b^2 = \frac{E}{m}$$ "
                     "\n Of course, you can also do other things, "
                     r"like $\theta = 90^\circ$ or $D^- \rightarrow D^0 \pi^- \pi^+$. "
                     "\n Sometimes it is necessary to escape commands with a double backslash, "
                     r"because e.g. \\theta will be interpreted as [tab]heta.")
    tntuple.SetAlias('Check', "This is the check text.")
    tntuple.SetAlias('Contact', "Name of the contact person.")
    tntuple.SetAlias('MetaOptions', "shifter, some_meta_options")

    # Overwrite the former TNtuple if one was there
    tntuple.Write()

    # Gauss Histogram
    # ======================================

    gaus_h = ROOT.TH1F("gaus_histogram", " Gaus Histogram", 100, -3, 3)
    gaus_h.FillRandom("gaus", 500)

    add_properties(
        gaus_h,
        {
            'Description': "xlog",
            'Check': "Gaus Histogram Check",
            'Contact': "Gaus Histogram Contact",
            'MetaOptions': "shifter, logx, nostats"
        }
    )

    gaus_h.Write()

    # Warnings
    # ======================================

    warnings_h = ROOT.TH1F(
        "warnings_histogram",
        "Histogram with missing check and description",
        100, -3, 3
    )
    warnings_h.FillRandom("gaus", 500)

    add_properties(
        warnings_h,
        {
            'Description': "",
            'Check': "",
            'Contact': "Kilian Lieret, Kilian.Lieret@campus.lmu.de",
            'MetaOptions': "shifter, logx, nostats"
        }
    )

    warnings_h.Write()

    # Exp histograms
    # ======================================

    exp_h = ROOT.TH1F("exp_histogram", " Expert Histogram", 100, 0, 10)

    exp_fn = ROOT.TF1("exp_fn", "exp(-x)", 0, 10)
    exp_h.FillRandom("exp_fn", 500)

    add_properties(
        exp_h,
        {
            'Description': "Expert Validation Plot",
            'Check': "Expert Histogram Check. Should only appear if expert button"
                     "was checked.",
            'Contact': "Expert Histogram Contact",
            'MetaOptions': "logy, nostats, C"
        }
    )

    exp_h.Write()

    # Gaus changing histogram
    # ======================================

    # set new random seed, to have changed a changed diagram for every
    # execution
    basf2.set_random_seed(datetime.now().microsecond)

    gaus_changing = ROOT.TH1F(
        "gaus_changing_histogram",
        "Gaus Changing Histogram",
        100, -5, 5
    )
    mean = ROOT.gRandom.Uniform(-1.0, 1.0)

    for i in range(500):
        v = ROOT.gRandom.Gaus(mean, 1.0)
        gaus_changing.Fill(v)

    add_properties(
        gaus_changing,
        {
            'Description': r"xlog ylog with stats. I can haz $\LaTeX$?",
            'Check': "Gaus Changing Histogram Check",
            'Contact': "Gaus Changing Histogram Contact",
            'MetaOptions': "shifter, logx, logy"
        }
    )

    gaus_changing.Write()

    # 2D histogram
    # ======================================
    hist2d = ROOT.TH2F("example_2d_histogram", "example_2d_title",
                       60, -3, 3, 60, -3, 3)

    mean = (0, 0)
    cov = [[1, 0], [0, 1]]
    for _ in range(500):
        x, y = np.random.multivariate_normal(mean, cov)
        hist2d.Fill(x, y)

    add_properties(
        hist2d,
        {
            'Description': "Some 2D Histogram",
            'Check': "Check For Something",
            'Contact': "Contact Someone",
            'MetaOptions': "shifter, contz"
        }
    )

    hist2d.Write()

    # TEfficiency plot
    # ======================================

    gaus_passed = ROOT.TH1F("gaus_passed", "gaus_passed", 50, -0.5, 49.5)
    gaus_total = ROOT.TH1F("gaus_total", "gaus_total", 50, -0.5, 49.5)

    for i in range(500):
        # shifted to more passed at larger positions
        ratio = float(i) / 500.0
        passed = ROOT.gRandom.Uniform(ratio * 0.45, 1.0)
        pos = ROOT.gRandom.Uniform(ratio * 30.0, 49.5)

        gaus_total.Fill(pos)
        if passed > 0.5:
            gaus_passed.Fill(pos)

    teff = ROOT.TEfficiency(gaus_passed, gaus_total)
    teff.SetName("TEfficiency")
    teff.SetTitle("Tefficiency")

    add_properties(
        teff,
        {
            'Description': "Efficiency plot of something",
            'Check': "Check For Something",
            'Contact': "Contact Someone",
            'MetaOptions': "shifter"
        }
    )

    teff.Write()

    # TGraphErrors
    # ======================================

    graph_err = ROOT.TGraphErrors()

    graph_err.Set(50)
    for i in range(50):
        # shifted to more passed at larger positions
        ratio = float(i) / 50.0
        passed = ROOT.gRandom.Uniform(ratio * 0.45, 1.0)

        graph_err.SetPoint(i, i + 1.0, passed)

    graph_err.SetName("TGraphErrors")
    graph_err.SetTitle("TGraphErrors")
    add_properties(
        graph_err,
        {
            'Description': "TGraphErrors plot of something",
            'Check': "Check For Something",
            'Contact': "Contact Someone",
            'MetaOptions': "shifter"
        }
    )

    graph_err.Write()

    # Html content
    # ======================================

    # generate some user-defined HTML
    html_content = ROOT.TNamed("This is a bold HTML tag", "<p><b>THIS IS USER'S HTML</b></p>")

    html_content.Write()

    tfile.Close()

    # Expert only
    # ======================================

    tfile = ROOT.TFile("validationTestPlotsExpertOnly.root", "RECREATE")
    gaus_h = ROOT.TH1F("gaus_histogram_exprt", " Gaus Histogram Expert", 100, -3, 3)
    gaus_h.FillRandom("gaus", 500)
    gaus_h.Write()
    tfile.Close()
