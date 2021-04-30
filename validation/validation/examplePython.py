#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>
<description>
This file contains example for generating validation plots using python
</description>
</header>
"""

import basf2
import array
import numpy as np
from ROOT import TEfficiency, TF1, TFile, TGraphErrors, TH1F, TH2F, TNamed, TNtuple, gRandom

basf2.set_random_seed(1337)

tfile = TFile("examplePython.root", "RECREATE")
tntuple = TNtuple("ntuple_test", "ntuple test", "x:y:z:k")

prefix = "examplePython"

array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0])
tntuple.Fill(array_of_values)

tntuple.SetAlias('Description', r"This is a description test. "
                 r"Lorem ipsum sit dolor amet.  We also support $\LaTeX$! "
                 "\n <br> \n For example, here is the infamous "
                 "Einstein-Pythagoras-theorem: \n "
                 r"$$a^2 + b^2 = \frac{E}{m}$$ "
                 "\n Of course, you can also "
                 r"do other things, like $\theta = 90^\circ$ or $D^- "
                 r"\rightarrow D^0 \pi^- \pi^+$. "
                 "\n Sometimes it is necessary to escape commands "
                 "with a double backslash, "
                 r"because e.g. \\theta will be interpreted as [tab]heta.")
tntuple.SetAlias('Check', "This is the check text.")
tntuple.SetAlias('Contact', "Name of the contact person.")
tntuple.SetAlias('MetaOptions', "some_meta_options")

# Overwrite the former TNtuple if one was there
tntuple.Write()

# Gauss Histogram
gausH = TH1F("gaus_histogram", prefix + " Gaus Histogram", 100, -3, 3)
gausH.FillRandom("gaus", 5000)

gausH.GetListOfFunctions().Add(TNamed('Description', "xlog"))
gausH.GetListOfFunctions().Add(TNamed('Check', "Gaus Histogram Check"))
gausH.GetListOfFunctions().Add(TNamed('Contact', "Gaus Histogram Contact"))
gausH.GetListOfFunctions().Add(TNamed('MetaOptions', "logx, nostats"))

gausH.Write()

gausH = TH1F("exp_histogram", prefix + " Exp Histogram", 100, 0, 10)

exp_fn = TF1("exp_fn", "exp(-x)", 0, 10)
gausH.FillRandom("exp_fn", 5000)

gausH.GetListOfFunctions().Add(TNamed('Description', "Expert Validation Plot"))
gausH.GetListOfFunctions().Add(TNamed('Check', "Exp Histogram Check"))
gausH.GetListOfFunctions().Add(TNamed('Contact', "Exp Histogram Contact"))
gausH.GetListOfFunctions().Add(TNamed(
    'MetaOptions', "logy, nostats, C"
))

gausH.Write()

# Example for a 2D histogram
hist2d = TH2F("example_2d_histogram", "example_2d_title",
              60, -3, 3, 60, -3, 3)

mean = (0, 0)
cov = [[1, 0], [0, 1]]
for i in range(10000):
    x, y = np.random.multivariate_normal(mean, cov)
    hist2d.Fill(x, y)

hist2d.GetListOfFunctions().Add(TNamed('Description', "Some 2D Histogram"))
hist2d.GetListOfFunctions().Add(TNamed('Check', "Check For Something"))
hist2d.GetListOfFunctions().Add(TNamed('Contact', "Contact Someone"))
hist2d.GetListOfFunctions().Add(TNamed('MetaOptions', "contz"))

hist2d.Write()

# add TEfficiency plot
gaus_passed = TH1F("gaus_passed", "gaus_passed", 50, -0.5, 49.5)
gaus_total = TH1F("gaus_total", "gaus_total", 50, -0.5, 49.5)

for i in range(5000):
    # shifted to more passed at larger positions
    ratio = float(i) / 5000.0
    passed = gRandom.Uniform(ratio * 0.45, 1.0)
    pos = gRandom.Uniform(ratio * 30.0, 49.5)
    gaus_total.Fill(pos)
# fixme
if passed > 0.5:
    gaus_passed.Fill(pos)

teff = TEfficiency(gaus_passed, gaus_total)
teff.SetName("TEfficiency")
teff.GetListOfFunctions().Add(TNamed('Description', "Efficiency plot of something"))
teff.GetListOfFunctions().Add(TNamed('Check', "Check For Something"))
teff.GetListOfFunctions().Add(TNamed('Contact', "Contact Someone"))
teff.GetListOfFunctions().Add(TNamed('MetaOptions', ""))

teff.Write()

# add TGraphErrors plot
graph_err = TGraphErrors()

graph_err.Set(50)
for i in range(50):
    # shifted to more passed at larger positions
    ratio = float(i) / 50.0
    passed = gRandom.Uniform(ratio * 0.45, 1.0)

# fixme
graph_err.SetPoint(i, i + 1.0, passed)

graph_err.SetName("TGraphErrors")
graph_err.GetListOfFunctions().Add(TNamed('Description', "TGraphErrors plot of something"))
graph_err.GetListOfFunctions().Add(TNamed('Check', "Check For Something"))
graph_err.GetListOfFunctions().Add(TNamed('Contact', "Contact Someone"))
graph_err.GetListOfFunctions().Add(TNamed('MetaOptions', ""))

graph_err.Write()

tfile.Close()
