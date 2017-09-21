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

h_born = ROOT.TH1F('h_born', 'Born cross section', 100, 6.1, 10.58)
h_born.GetXaxis().SetTitle('M_{J/#psi#eta_{c}}^{cutoff}, GeV/c^{2}')
h_born.GetYaxis().SetTitle('#sigma_{e^{+} e^{-} #rightarrow J/#psi #eta_{c}}, arbitrary units')

h_helicity = ROOT.TH1F('h_helicity', 'Virtual #gamma helicity angle', 20, -1, 1)
h_helicity.GetXaxis().SetTitle('cos#Theta')
h_helicity.GetYaxis().SetTitle('Events / 0.1')
h_helicity.SetMinimum(0)

n = data_tree.GetEntries()
for i in range(0, n):
    data_tree.GetEntry(i)
    h_born.Fill(data_tree.gamma_M)
    p_gamma = ROOT.TLorentzVector(data_tree.gamma_P4[0], data_tree.gamma_P4[1],
                                  data_tree.gamma_P4[2], data_tree.gamma_P4[3])
    p_jpsi = ROOT.TLorentzVector(data_tree.gamma_Jpsi_P4[0], data_tree.gamma_Jpsi_P4[1],
                                 data_tree.gamma_Jpsi_P4[2], data_tree.gamma_Jpsi_P4[3])
    v_boost = -p_gamma.BoostVector()
    p_jpsi.Boost(v_boost)
    h_helicity.Fill(math.cos(p_gamma.Vect().Angle(p_jpsi.Vect())))

s = 0
ecms = 10.58
for i in range(100, 0, -1):
    bc = h_born.GetBinContent(i)
    s += bc
    p = s / n
    q = 1.0 - p
    r = born_measured_ratio(ecms, ecms - h_born.GetBinLowEdge(i))
    h_born.SetBinContent(i, s / r / n)
    h_born.SetBinError(i, math.sqrt(n * p * q) / r / n)

contact = 'Kirill Chilikin (chilikin@lebedev.ru)'
l = h_born.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'Born cross section calculated from measured cross section'))
l.Add(ROOT.TNamed('Check', 'Should be consistent with constant'))
l.Add(ROOT.TNamed('Contact', contact))
l = h_helicity.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'Virtual photon helicity angle'))
l.Add(ROOT.TNamed('Check', 'Should be consistent with constant'))
l.Add(ROOT.TNamed('Contact', contact))

output_file.cd()
h_born.Write()
h_helicity.Write()
output_file.Close()
