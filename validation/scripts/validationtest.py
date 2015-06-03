#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This file will generate various output into a root-file to have
a fast turn-around during development
"""

import basf2
import array
from datetime import datetime
from ROOT import TFile, TNtuple, TH1F, TF1, TRandom3, gRandom


def generateTestPlots(prefix):
    tntuple = TNtuple("ntuple_test" + prefix, "ntuple test" + prefix, "x:y:z:k")

    array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0])
    tntuple.Fill(array_of_values)

    tntuple.SetAlias('Description', "description text")
    tntuple.SetAlias('Check', "check text")
    tntuple.SetAlias('Contact', "contact text")

    # Overwrite the former TNtuple if one was there
    tntuple.Write()

    gausH = TH1F("gaus_histogram" + prefix, "Gaus Histogram" + prefix, 100, -3, 3)
    gausH.FillRandom("gaus", 5000)

    tntuple.SetAlias('Description', "Gaus Histogram text")
    tntuple.SetAlias('Check', "Gaus Histogram check text")
    tntuple.SetAlias('Contact', "Gaus Histogram contact text")

    gausH.Write()

    gausH = TH1F("exp_histogram" + prefix, "Exp Histogram" + prefix, 100, 0, 10)

    exp_fn = TF1("exp_fn", "exp(-x)", 0, 10)
    gausH.FillRandom("exp_fn", 5000)

    tntuple.SetAlias('Description', "Exp Histogram text")
    tntuple.SetAlias('Check', "Exp Histogram check text")
    tntuple.SetAlias('Contact', "Exp Histogram contact text")

    gausH.Write()

    # set new random seed, to have changed a changed diagram for every
    # execution
    basf2.set_random_seed(datetime.now().microsecond)

    gausH = TH1F("gaus_changing_histogram" + prefix, "Gaus Changing Histogram" + prefix, 100, -5, 5)
    mean = gRandom.Uniform(-1.0, 1.0)

    for i in range(5000):
        v = gRandom.Gaus(mean, 1.0)
        gausH.Fill(v)

    tntuple.SetAlias('Description', "Gaus Changing Histogram text")
    tntuple.SetAlias('Check', "Gaus Changing Histogram check text")
    tntuple.SetAlias('Contact', "Gaus Changing Histogram contact text")

    gausH.Write()
