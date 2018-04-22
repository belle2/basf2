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

nbins_ratio = 91
emin_ratio = 6.05
emax_ratio = 10.6
e_threshold = 6.077216
vacpol_coef = [
    1.0628848921570468,
    1.0630222501435538,
    1.0631583712375692,
    1.0632932870752236,
    1.0634270280964420,
    1.0635596236029659,
    1.0636911038563479,
    1.0638214919564521,
    1.0639508161489530,
    1.0640911763911554,
    1.0642426021238256,
    1.0643930465044915,
    1.0645425321383304,
    1.0646910829012752,
    1.0648387157899668,
    1.0649854532304037,
    1.0651313149251775,
    1.0652763219764185,
    1.0654204887222767,
    1.0655638350494281,
    1.0657063782276197,
    1.0658481370304009,
    1.0659891235597700,
    1.0661293555636853,
    1.0662688482593514,
    1.0664076184528672,
    1.0665456763532895,
    1.0666830378959860,
    1.0668197165557802,
    1.0669728086606438,
    1.0671423358423944,
    1.0673112348835889,
    1.0674795135759751,
    1.0676471855363563,
    1.0678142640605996,
    1.0679807559683534,
    1.0681466739443513,
    1.0683120303864380,
    1.0684768312444710,
    1.0686410883688278,
    1.0688048112946322,
    1.0689680113655782,
    1.0691306935088203,
    1.0692928706527736,
    1.0694545452046977,
    1.0696157337719090,
    1.0697764383327544,
    1.0699366709637401,
    1.0700964353013993,
    1.0702369187285552,
    1.0703581196112877,
    1.0704788617065120,
    1.0705991500549692,
    1.0707189957188554,
    1.0708384034084868,
    1.0709573776751666,
    1.0710759291100751,
    1.0711940619641145,
    1.0713117824086880,
    1.0714290944104019,
    1.0715460079976491,
    1.0716625248052389,
    1.0717786546050720,
    1.0718944008474305,
    1.0720097668616437,
    1.0721247620626524,
    1.0722393895512141,
    1.0723536543855732,
    1.0724675594488726,
    1.0725811137265711,
    1.0726943178288948,
    1.0728071805444170,
    1.0729197043593588,
    1.0730318937365730,
    1.0731437509784216,
    1.0732552845114489,
    1.0733664964658656,
    1.0734773888878144,
    1.0735879699573529,
    1.0736982394896657,
    1.0738082055112450,
    1.0739178697580838,
    1.0740272359656267,
    1.0741363077987014,
    1.0742450867795019,
    1.0743535805843314,
    1.0744617885288132,
    1.0745697181614047,
    1.0746773707456787,
    1.0749101058390413,
    1.0752680569983371]


def ratio_measured_ratio(ecms, ecut):
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


def cross_section_ee_mumu(ecms):
    alpha = 7.2973525664e-3
    mmu = 0.1056583715
    conv = 0.389379338 * 1e12  # Gev^2 * fb
    s = ecms * ecms
    if (ecms < 2.0 * mmu):
        return 0
    return 4.0 * math.pi * alpha * alpha / (3.0 * s) * conv * \
        math.sqrt(1.0 - 4.0 * mmu * mmu / s) * (1.0 + 2.0 * mmu * mmu / s)


def effective_luminosity(ecms, e):
    alpha = 7.2973525664e-3
    me = 0.510998928e-3
    pi = math.pi
    L = 2.0 * math.log(ecms / me)
    beta = 2.0 * alpha / pi * (L - 1)
    x = 1.0 - pow(e / ecms, 2)
    lum = beta * pow(x, beta - 1) * \
        (1. + alpha / pi * (pi * pi / 3 - 1. / 2) + 3. * beta / 4 -
         beta * beta / 24 * (L / 3 + 2. * pi * pi - 37. / 4)) - \
        beta * (1. - x / 2) + beta * beta / 8 * \
        (4. * (2. - x) * math.log(1. / x) + (1. + 3. * pow(1. - x, 2)) / x *
         math.log(1. / (1. - x)) - 6 + x)
    # Jacobian dx -> dE
    lum = lum * 2.0 * e / ecms / ecms
    return lum


def effective_luminosity_integral(ecms, emin, emax):
    if (emax < ecms):
        npoints = 1000
        intg = 0
        for i in range(0, npoints):
            e = emin + (emax - emin) / npoints * (float(i) + 0.5)
            if (e > e_threshold):
                intg = intg + effective_luminosity(ecms, e)
        intg = intg / npoints
        return intg
    # The value of effective_luminosity -> infinity as ecut -> 0,
    # but its integral converges => the approximate formula is used.
    elif (emin < ecms):
        ecut = ecms - emin
        return ratio_measured_ratio(ecms, ecut) / (emax - emin)
    else:
        return 0


input_file = ROOT.TFile('PhokharaEvtgenAnalysis.root')
tree = input_file.Get('tree')
output_file = ROOT.TFile('PhokharaEvtgen.root', 'recreate')

h_mgamma_exp = ROOT.TH1F('h_mgamma_exp', 'Virtual #gamma mass distribution (theory)', nbins_ratio, emin_ratio, emax_ratio)

h_ratio = ROOT.TH1F('h_ratio', 'Virtual #gamma mass distribution / expectation', nbins_ratio, emin_ratio, emax_ratio)
h_ratio.GetXaxis().SetTitle('M_{J/#psi#eta_{c}}^{cutoff}, GeV/c^{2}')
h_ratio.GetYaxis().SetTitle('#sigma_{e^{+} e^{-} #rightarrow J/#psi #eta_{c}}, arbitrary units')

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
    h_ratio.Fill(m)
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

for i in range(nbins_ratio, 0, -1):
    emin = emin_ratio + (emax_ratio - emin_ratio) / nbins_ratio * (i - 1)
    emax = emin_ratio + (emax_ratio - emin_ratio) / nbins_ratio * i
    l = effective_luminosity_integral(ecms, emin, emax)
    xs = cross_section_ee_mumu(h_ratio.GetBinLowEdge(i)) * vacpol_coef[i - 1]
    h_mgamma_exp.SetBinContent(i, xs * l)

h_mgamma_exp.Scale(n / h_mgamma_exp.Integral())

for i in range(nbins_ratio, 0, -1):
    val = h_ratio.GetBinContent(i)
    err = h_ratio.GetBinError(i)
    exp = h_mgamma_exp.GetBinContent(i)
    h_ratio.SetBinContent(i, val / exp)
    h_ratio.SetBinError(i, err / exp)

contact = 'Kirill Chilikin (chilikin@lebedev.ru)'
l = h_ratio.GetListOfFunctions()
l.Add(ROOT.TNamed('Description', 'Number of events / theoretical expectation'))
l.Add(ROOT.TNamed('Check', 'Should be consistent with 1'))
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
h_ratio.Write()
h_helicity_gamma.Write()
h_helicity_jpsi.Write()
output_file.Close()
