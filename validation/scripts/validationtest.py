#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This file will generate various output into a root-file to have
a fast turn-around during development
"""

import basf2
import array
from datetime import datetime
from ROOT import TFile, TNtuple, TH1F, TF1, TRandom3, gRandom, TNamed


def generateTestPlots(prefix):
    tntuple = TNtuple("ntuple_test", "ntuple test", "x:y:z:k")

    array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0])
    tntuple.Fill(array_of_values)

    tntuple.SetAlias('Description', "This is a description test. "
                     "Lorem ipsum sit dolor amet.  We also support $\LaTeX$! "
                     "\n <br> \n For example, here is the infamous "
                     "Einstein-Pythagoras-theorem: \n "
                     "$$a^2 + b^2 = \\frac{E}{m}$$ \n Of course, you can also"
                     "do other things, like $\\theta = 90^\circ$ or $D^- "
                     "\\rightarrow D^0 \pi^- \pi^+$. \n Sometimes it is "
                     "necessary to escape commands with a double backslash, "
                     "because e.g. \\theta will be interpreted as [tab]heta.")
    tntuple.SetAlias('Check', "This is the check text.")
    tntuple.SetAlias('Contact', "Name of the contact person.")

    # Overwrite the former TNtuple if one was there
    tntuple.Write()

    gausH = TH1F("gaus_histogram", prefix + " Gaus Histogram", 100, -3, 3)
    gausH.FillRandom("gaus", 5000)

    gausH.GetListOfFunctions().Add(TNamed('Description', "Gaus Histogram Description"))
    gausH.GetListOfFunctions().Add(TNamed('Check', "Gaus Histogram Check"))
    gausH.GetListOfFunctions().Add(TNamed('Contact', "Gaus Histogram Contact"))

    gausH.Write()

    gausH = TH1F("exp_histogram", prefix + " Exp Histogram", 100, 0, 10)

    exp_fn = TF1("exp_fn", "exp(-x)", 0, 10)
    gausH.FillRandom("exp_fn", 5000)

    gausH.GetListOfFunctions().Add(TNamed('Description', "Exp Histogram Description"))
    gausH.GetListOfFunctions().Add(TNamed('Check', "Exp Histogram Check"))
    gausH.GetListOfFunctions().Add(TNamed('Contact', "Exp Histogram Contact"))

    gausH.Write()

    # set new random seed, to have changed a changed diagram for every
    # execution
    basf2.set_random_seed(datetime.now().microsecond)

    gausH = TH1F("gaus_changing_histogram", prefix + " Gaus Changing Histogram", 100, -5, 5)
    mean = gRandom.Uniform(-1.0, 1.0)

    for i in range(5000):
        v = gRandom.Gaus(mean, 1.0)
        gausH.Fill(v)

    gausH.GetListOfFunctions().Add(TNamed('Description', "Gaus Changing Histogram Description"))
    gausH.GetListOfFunctions().Add(TNamed('Check', "Gaus Changing Histogram Check"))
    gausH.GetListOfFunctions().Add(TNamed('Contact', "Gaus Changing Histogram Contact"))

    gausH.Write()
