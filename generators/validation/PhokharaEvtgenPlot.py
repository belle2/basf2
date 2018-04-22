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
tree = input_file.Get('tree')
output_file = ROOT.TFile('PhokharaEvtgen.root', 'recreate')

h_born = ROOT.TH1F('h_born', 'Born cross section', 100, 6.1, 10.58)
h_born.GetXaxis().SetTitle('M_{J/#psi#eta_{c}}^{cutoff}, GeV/c^{2}')
h_born.GetYaxis().SetTitle('#sigma_{e^{+} e^{-} #rightarrow J/#psi #eta_{c}}, arbitrary units')

h_helicity_gamma = ROOT.TH1F('h_helicity_gamma', 'Virtual #gamma helicity angle', 20, -1, 1)
h_helicity_gamma.GetXaxis().SetTitle('cos#theta_{#gamma}')
h_helicity_gamma.GetYaxis().SetTitle('Events / 0.1')
h_helicity_gamma.SetMinimum(0)

h_helicity_jpsi = ROOT.TH1F('h_helicity_jpsi', 'J/#psi helicity angle', 20, -1, 1)
h_helicity_jpsi.GetXaxis().SetTitle('cos#theta_{J/#psi}')
h_helicity_jpsi.GetYaxis().SetTitle('Events / 0.1')
h_helicity_jpsi.SetMinimum(0)

n = tree.GetEntries()
tree.GetEntry(0)
ecms = tree.ecms
for i in range(0, n):
    tree.GetEntry(i)
    m = math.sqrt(tree.gamma_e * tree.gamma_e - tree.gamma_px * tree.gamma_px -
                  tree.gamma_py * tree.gamma_py - tree.gamma_pz * tree.gamma_pz)
    h_born.Fill(m)
    p_gamma = ROOT.TLorentzVector(tree.gamma_px, tree.gamma_py,
                                  tree.gamma_pz, tree.gamma_e)
    p_jpsi = ROOT.TLorentzVector(tree.jpsi_px, tree.jpsi_py,
                                 tree.jpsi_pz, tree.jpsi_e)
    p_lepton = ROOT.TLorentzVector(tree.lepton_px, tree.lepton_py,
                                   tree.lepton_pz, tree.lepton_e)
    v_boost_gamma = -p_gamma.BoostVector()
    v_boost_jpsi = -p_jpsi.BoostVector()
    p_jpsi.Boost(v_boost_gamma)
    h_helicity_gamma.Fill(-math.cos(p_gamma.Vect().Angle(p_jpsi.Vect())))
    p_lepton.Boost(v_boost_jpsi)
    p_gamma.Boost(v_boost_jpsi)
    h_helicity_jpsi.Fill(-math.cos(p_gamma.Vect().Angle(p_lepton.Vect())))

s = 0
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
l = h_helicity_gamma.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'Virtual photon helicity angle'))
l.Add(ROOT.TNamed('Check', 'Should be distributed as (1 + cos^2 theta)'))
l.Add(ROOT.TNamed('Contact', contact))
l = h_helicity_jpsi.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'J/psi helicity angle'))
l.Add(ROOT.TNamed('Check', 'Should be distributed as (1 + cos^2 theta)'))
l.Add(ROOT.TNamed('Contact', contact))

output_file.cd()
h_born.Write()
h_helicity_gamma.Write()
h_helicity_jpsi.Write()
output_file.Close()
