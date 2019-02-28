#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>validationTestNTuple.root</output>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>
</header>
"""


import array
import ROOT

tfile = ROOT.TFile("validationTestNTuple.root", "RECREATE")

# Default precision
# =================

tntuple = ROOT.TNtuple("ntuple_test", "ntuple test", "x:y:z:k")

array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0, 12, 15, 16])
tntuple.Fill(array_of_values)

tntuple.SetAlias('Description', "Test default precision")
tntuple.SetAlias('Check', "Should display the default of 4 digits.")
tntuple.SetAlias('Contact', "Kilian Lieret, Kilian.Lieret@campus.lmu.de")
tntuple.SetAlias('MetaOptions', "some_meta_options")

tntuple.Write()

# High precision
# =================

tntuple_hp = ROOT.TNtuple("ntuple_test_hp", "ntuple test", "x:y:z:k")

array_of_values = array.array('f', [23.4, 4.4, 5.12, -23.0, 12, 15, 16])
tntuple_hp.Fill(array_of_values)

tntuple_hp.SetAlias('Description', "Test higher precision")
tntuple_hp.SetAlias('Check', "Should display 8 digits per float.")
tntuple_hp.SetAlias('Contact', "Kilian Lieret, Kilian.Lieret@campus.lmu.de")
tntuple_hp.SetAlias('MetaOptions', "float-precision=8")

tntuple_hp.Write()

# Closing

tfile.Close()
