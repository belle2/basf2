#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>PhokharaEvtgenAnalysis.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Analysis of e+ e- -> J/psi eta_c events.</description>
</header>
"""

from basf2 import *
import ROOT
import math


def born_measured_ratio(ecms, ecut):
    alpha = 7.2973525664e-3
    me = 0.510998928e-3
    riemann_zeta_3 = 1.2020569032
    pi = math.pi
    l = math.log(ecms / 2 / ecut)
    L = 2.0 * math.log(ecms / me)
    r1 = -2.0 * l * (L - 1) + 1.5 * L + pi * pi / 3 - 2
    r2 = 0.5 * pow(-2.0 * l * (L - 1), 2) + \
        (1.5 * L + pi * pi / 3 - 2) * (-2.0 * l * (L - 1)) + L * L * (-l / 3 + 11. / 8 - pi * pi / 3) + \
        L * (2.0 * l * l / 3 + 10. * l / 9 - 203. / 48 + 11. * pi * pi / 12 + 3.0 * riemann_zeta_3) - \
        (4. * l * l * l / 9 + 10. * l * l / 9 + 2. * l / 9 * (28. / 3 - pi * pi)) - \
        (pow(L - 2. * l, 3) / 18 - 5. / 18 * pow(L - 2. * l, 2) + (28. / 3 - pi * pi) * (L - 2. * l) / 9)
    r = 1. + alpha / pi * r1 + alpha * alpha / pi / pi * r2
    return r


input_file = ROOT.TFile('PhokharaEvtgenAnalysis.root')
data_tree = input_file.Get('tree')
output_file = ROOT.TFile('PhokharaEvtgen.root', 'recreate')

h_gamma_mass = ROOT.TH1F('gamma_mass', 'Born cross section', 100, 6.1, 10.58)
h_gamma_mass.GetXaxis().SetTitle('M_{J/#psi#eta_{c}}^{cutoff}, GeV/c^{2}')
h_gamma_mass.GetYaxis().SetTitle('#sigma_{e^{+} e^{-} #rightarrow J/#psi #eta_{c}}, arbitrary units')
n = data_tree.GetEntries()
for i in range(0, n):
    data_tree.GetEntry(i)
    h_gamma_mass.Fill(data_tree.gamma_M)
s = 0
ecms = 10.58
for i in range(100, 0, -1):
    bc = h_gamma_mass.GetBinContent(i)
    s += bc
    p = s / n
    q = 1.0 - p
    r = born_measured_ratio(ecms, ecms - h_gamma_mass.GetBinLowEdge(i))
    h_gamma_mass.SetBinContent(i, s / r)
    h_gamma_mass.SetBinError(i, math.sqrt(n * p * q) / r)

contact = 'Kirill Chilikin (chilikin@lebedev.ru)'
l = h_gamma_mass.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'Born cross section calculated from measured cross section'))
l.Add(ROOT.TNamed('Check', 'Should be consistent with constant'))
l.Add(ROOT.TNamed('Contact', contact))

output_file.cd()
h_gamma_mass.Write()
output_file.Close()
