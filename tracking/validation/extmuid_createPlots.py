#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#     Create ext and muid validation plots and store them       #
#     in a root file                                            #
#                                                               #
#    written by Leo Piilonen, VT                                #
#    piilonen@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <input>muon-ExtMuidValidation.root,
           pion-ExtMuidValidation.root</input>
    <contact>piilonen@vt.edu</contact>
    <description>Create validation plots for Ext and Muid</description>
</header>
"""

import ROOT

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gROOT.SetBatch(True)

from ROOT import TFile, TChain, TTree, TH1F, TH2F, TCanvas, TNamed, gStyle
import sys
import math
import numpy as np
from optparse import OptionParser

# contact person information
# is added to the plot descriptions
CONTACT_PERSON = {'Name': 'Leo Piilonen',
                  'Email': 'piilonen@vt.edu'}


def main():
    """Create validation plots for Ext and Muid"""

    option_parser = OptionParser()
    option_parser.add_option('-i', '--input-file', dest='input_file',
                             default='../*on-ExtMuidValidation.root',
                             help='Root file with Ext and Muid validation data.'
                             )
    option_parser.add_option('-o', '--output-file', dest='output_file',
                             default='ExtMuidValidation.root',
                             help='Root file with Ext and Muid validation histograms.')

    (options, args) = option_parser.parse_args()

    # load chain of input files
    file_chain = TChain('tree')
    file_chain.Add(options.input_file)

    # for file in file_chain:

    number_entries = 0
    try:
        number_entries = file_chain.GetEntries()
    except AttributeError:
        print('Could not load input file(s) %s.' % options.input_file)

    if number_entries == 0:
        print('Data tree is empty or does not exist in file(s) %s. Exit.' % options.input_file)
        sys.exit(0)

    # open the output root file
    output_root_file = TFile(options.output_file, 'recreate')

    # create and draw histograms
    gStyle.SetOptStat(0)
    draw_exthits(file_chain)
    draw_likelihoods(file_chain)

    # close the output file
    output_root_file.Write()
    output_root_file.Close()


def draw_exthits(file_chain):
    """
    Draw the ExtHit-related distributions.
    """

    # NOTE: *.Draw() must precede *.GetListOfFunctions().Add() or the latter will be discarded!
    detectorID = TH1F('DetectorID', 'Detector ID for ExtHits', 8, -0.5, 7.5)
    file_chain.Draw('ExtHits.m_DetectorID&0x0F>>DetectorID', '')
    detectorID.GetXaxis().SetTitle('0=undefined, 1=PXD, 2=SVD, 3=CDC, 4=TOP, 5=ARICH, 6=ECL, 7=KLM')
    detectorID.GetListOfFunctions().Add(TNamed('Description', "0=undefined, 1=PXD, 2=SVD, 3=CDC, 4=TOP, 5=ARICH, 6=ECL, 7=KLM"))
    detectorID.GetListOfFunctions().Add(TNamed('Check', "ECL > (KLM ~ TOP) >> ARICH; others ~ 0"))
    detectorID.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    detectorID.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    detectorID.Write()

    tof = TH1F('TOF', 'Time of Flight for non-KLM ExtHits', 100, 0.0, 25.0)
    file_chain.Draw('ExtHits.m_TOF>>TOF', '(ExtHits.m_DetectorID&0x0F)!=7')
    tof.GetXaxis().SetTitle('t (ns)')
    tof.GetListOfFunctions().Add(TNamed('Description', "Time of flight along track from IP to each ExtHit"))
    tof.GetListOfFunctions().Add(TNamed('Check', "Dominant peak at ~4.5 ns, secondary peak at ~6 ns"))
    tof.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    tof.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    tof.Write()

    tofKLM = TH1F('TOFKLM', 'Time of Flight for KLM ExtHits', 100, 0.0, 25.0)
    file_chain.Draw('ExtHits.m_TOF>>TOFKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    tofKLM.GetXaxis().SetTitle('t (ns)')
    tofKLM.GetListOfFunctions().Add(TNamed('Description', "Time of flight along track from IP to each ExtHit"))
    tofKLM.GetListOfFunctions().Add(TNamed('Check', "Broad rising distribution at 7-11 ns, then falling to ~17 ns"))
    tofKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    tofKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    tofKLM.Write()

    r = TH1F('r', 'r for non-KLM ExtHits', 40, 0.0, 200.0)
    file_chain.Draw('ExtHits.getPosition().Perp()>>r', '(ExtHits.m_DetectorID&0x0F)!=7')
    r.GetXaxis().SetTitle('r (cm)')
    r.GetListOfFunctions().Add(TNamed('Description', "Radial position in transverse plane of each ExtHit"))
    r.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at 120 cm; broad peak at 140 cm"))
    r.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    r.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    r.Write()

    z = TH1F('z', 'z for non-KLM ExtHits', 100, -200.0, 300.0)
    file_chain.Draw('ExtHits.getPosition().Z()>>z', '(ExtHits.m_DetectorID&0x0F)!=7')
    z.GetXaxis().SetTitle('z (cm)')
    z.GetListOfFunctions().Add(TNamed('Description', "Axial position of each ExtHit"))
    z.GetListOfFunctions().Add(TNamed('Check', "Broad peak centered at 0 cm; sharp dip at 0; secondary peak at 170"))
    z.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    z.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    z.Write()

    rKLM = TH1F('rKLM', 'r for KLM ExtHits', 50, 100.0, 350.0)
    file_chain.Draw('ExtHits.getPosition().Perp()>>rKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    rKLM.GetXaxis().SetTitle('r (cm)')
    rKLM.GetListOfFunctions().Add(TNamed('Description', "Radial position in transverse plane of each ExtHit"))
    rKLM.GetListOfFunctions().Add(TNamed('Check', "Low shoulder below 200 cm (EKLM); comb-like pattern above 200 cm (BKLM)"))
    rKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    rKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    rKLM.Write()

    zKLM = TH1F('zKLM', 'z for KLM ExtHits', 140, -300.0, 400.0)
    file_chain.Draw('ExtHits.getPosition().Z()>>zKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    zKLM.GetXaxis().SetTitle('z (cm)')
    zKLM.GetListOfFunctions().Add(TNamed('Description', "Axial position of each ExtHit"))
    zKLM.GetListOfFunctions().Add(TNamed('Check', "Comb-like pattern at z<-180 and z>280 cm (EKLM); broad peak at 0 (BKLM)"))
    zKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    zKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    zKLM.Write()

    xy = TH2F('xy', 'y vs x for non-KLM ExtHits', 100, -200.0, 200.0, 100, -200.0, 200.0)
    file_chain.Draw('ExtHits.getPosition().Y():ExtHits.getPosition().X()>>xy', '(ExtHits.m_DetectorID&0x0F)!=7')
    xy.GetXaxis().SetTitle('x (cm)')
    xy.GetYaxis().SetTitle('y (cm)')
    xy.GetListOfFunctions().Add(TNamed('Description', "Position projected into transverse plane of each ExtHit"))
    xy.GetListOfFunctions().Add(TNamed('Check', "Cylindrical, with most hits in ECL and TOP"))
    xy.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    xy.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    xy.Write()

    xz = TH2F('xz', 'x vs z for non-KLM ExtHits', 125, -200.0, 300.0, 100, -200.0, 200.0)
    file_chain.Draw('ExtHits.getPosition().X():ExtHits.getPosition().Z()>>xz', '(ExtHits.m_DetectorID&0x0F)!=7')
    xz.GetXaxis().SetTitle('z (cm)')
    xz.GetYaxis().SetTitle('x (cm)')
    xz.GetListOfFunctions().Add(TNamed('Description', "Position projected into x-z plane of each ExtHit"))
    xz.GetListOfFunctions().Add(TNamed('Check', " "))
    xz.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    xz.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    xz.Write()

    yz = TH2F('yz', 'y vs z for non-KLM ExtHits', 125, -200.0, 300.0, 100, -200.0, 200.0)
    file_chain.Draw('ExtHits.getPosition().Y():ExtHits.getPosition().Z()>>yz', '(ExtHits.m_DetectorID&0x0F)!=7')
    yz.GetXaxis().SetTitle('z (cm)')
    yz.GetYaxis().SetTitle('y (cm)')
    yz.GetListOfFunctions().Add(TNamed('Description', "Position projected into y-z plane of each ExtHit"))
    yz.GetListOfFunctions().Add(TNamed('Check', " "))
    yz.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    yz.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    yz.Write()

    xyKLM = TH2F('xyKLM', 'y vs x for KLM ExtHits', 140, -350.0, 350.0, 140, -350.0, 350.0)
    file_chain.Draw('ExtHits.getPosition().Y():ExtHits.getPosition().X()>>xyKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    xyKLM.GetXaxis().SetTitle('x (cm)')
    xyKLM.GetYaxis().SetTitle('y (cm)')
    xyKLM.GetListOfFunctions().Add(TNamed('Description', "Position projected into transverse plane of each ExtHit"))
    xyKLM.GetListOfFunctions().Add(TNamed('Check', "Octagonal (BKLM) and cylindrical (EKLM)"))
    xyKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    xyKLM.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    xyKLM.Write()

    xzKLM = TH2F('xzKLM', 'x vs z for KLM ExtHits', 140, -300.0, 400.0, 140, -350.0, 350.0)
    file_chain.Draw('ExtHits.getPosition().X():ExtHits.getPosition().Z()>>xzKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    xzKLM.GetXaxis().SetTitle('z (cm)')
    xzKLM.GetYaxis().SetTitle('x (cm)')
    xzKLM.GetListOfFunctions().Add(TNamed('Description', "Position projected into x-z plane of each ExtHit"))
    xzKLM.GetListOfFunctions().Add(TNamed('Check', " "))
    xzKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    xzKLM.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    xzKLM.Write()

    yzKLM = TH2F('yzKLM', 'y vs z for KLM ExtHits', 140, -300.0, 400.0, 140, -350.0, 350.0)
    file_chain.Draw('ExtHits.getPosition().Y():ExtHits.getPosition().Z()>>yzKLM', '(ExtHits.m_DetectorID&0x0F)==7')
    yzKLM.GetXaxis().SetTitle('z (cm)')
    yzKLM.GetYaxis().SetTitle('y (cm)')
    yzKLM.GetListOfFunctions().Add(TNamed('Description', "Position projected into y-z plane of each ExtHit"))
    yzKLM.GetListOfFunctions().Add(TNamed('Check', " "))
    yzKLM.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    yzKLM.GetListOfFunctions().Add(TNamed('MetaOptions', "box, expert"))
    yzKLM.Write()


def draw_likelihoods(file_chain):
    """
    Draw the Muid likelihood-based distributions.
    """

    outcome = TH1F('Outcome', 'Outcome', 5, -0.5, 4.5)

    llBins = 120
    llMin = -60.0
    llMax = 0.0
    llMu_mu = TH1F('LLmu-muons', 'logL(mu) for true muons', llBins, llMin, llMax)
    llMu_pi = TH1F('LLmu-pions', 'logL(mu) for true pions', llBins, llMin, llMax)
    llPi_mu = TH1F('LLpi-muons', 'logL(pi) for true muons', llBins, llMin, llMax)
    llPi_pi = TH1F('LLpi-pions', 'logL(pi) for true pions', llBins, llMin, llMax)
    llDiff_mu = TH1F('LLdiff-muons', 'logL(mu)-logL(pi) for true muons', llBins * 2, llMin, -llMin)
    llDiff_pi = TH1F('LLdiff-pions', 'logL(mu)-logL(pi) for true pions', llBins * 2, llMin, -llMin)

    layerBins = 15
    layerMin = -0.5
    layerMax = 14.5
    barrelLayer_mu = TH1F('BarrelLayer-muons', 'Last barrel layer with hit for true muons', layerBins, layerMin, layerMax)
    barrelLayer_pi = TH1F('BarrelLayer-pions', 'Last barrel layer with hit for true pions', layerBins, layerMin, layerMax)
    endcapLayer_mu = TH1F('EndcapLayer-muons', 'Last endcap layer with hit for true muons', layerBins, layerMin, layerMax)
    endcapLayer_pi = TH1F('EndcapLayer-pions', 'Last endcap layer with hit for true pions', layerBins, layerMin, layerMax)

    diffLayerBins = 20
    diffLayerMin = -0.5
    diffLayerMax = 19.5
    diffLayer_mu = TH1F('DiffLayer-muons', 'Range difference for true muons', diffLayerBins, diffLayerMin, diffLayerMax)
    diffLayer_pi = TH1F('DiffLayer-pions', 'Range difference for true pions', diffLayerBins, diffLayerMin, diffLayerMax)

    ndofBins = 32
    ndofMin = -0.5
    ndofMax = 31.5
    ndof_mu = TH1F('Ndof-muons', 'Degrees of freedom for true muons', ndofBins, ndofMin, ndofMax)
    ndof_pi = TH1F('Ndof-pions', 'Degrees of freedom for true pions', ndofBins, ndofMin, ndofMax)

    rchisqBins = 50
    rchisqMin = 0.0
    rchisqMax = 10.0
    rchisq_mu = TH1F('rchisq-muons', 'Reduced chi-squared for true muons', rchisqBins, rchisqMin, rchisqMax)
    rchisq_pi = TH1F('rchisq-pions', 'Reduced chi-squared for true pions', rchisqBins, rchisqMin, rchisqMax)

    momentumBins = 25  # for efficiency
    momentumBins2 = 5  # for fake rate (low statistics)
    momentumMin = 0.0
    momentumMax = 5.0
    efficiency_momentum = TH1F('Eff-momentum', 'Muon efficiency vs momentum', momentumBins, momentumMin, momentumMax)
    efficiency_momentum_denom = TH1F('Eff-momentum-denom', 'Muon efficiency vs momentum', momentumBins, momentumMin, momentumMax)
    fakerate_momentum = TH1F('FakeRate-momentum', 'Pion fake rate vs momentum', momentumBins2, momentumMin, momentumMax)
    fakerate_momentum_denom = TH1F('FakeRate-momentum-denom', 'Pion fake rate vs momentum', momentumBins2, momentumMin, momentumMax)

    thetaBins = 35  # for efficiency
    thetaBins2 = 7  # for fake rate (low statistics)
    thetaMin = 10.0
    thetaMax = 150.0
    efficiency_theta = TH1F('Eff-theta', 'Muon efficiency vs theta', thetaBins, thetaMin, thetaMax)
    efficiency_theta_denom = TH1F('Eff-theta-denom', 'Muon efficiency vs theta', thetaBins, thetaMin, thetaMax)
    fakerate_theta = TH1F('FakeRate-theta', 'Pion fake rate vs theta', thetaBins2, thetaMin, thetaMax)
    fakerate_theta_denom = TH1F('FakeRate-theta-denom', 'Pion fake rate vs theta', thetaBins2, thetaMin, thetaMax)

    phiBins = 36  # for efficiency
    phiBins2 = 8  # for fake rate (low statistics)
    phiMin = 0.0
    phiMax = 360.0
    efficiency_phi = TH1F('Eff-phi', 'Muon efficiency vs phi', phiBins, phiMin, phiMax)
    efficiency_phi_denom = TH1F('Eff-phi-denom', 'Muon efficiency vs phi', phiBins, phiMin, phiMax)
    fakerate_phi = TH1F('FakeRate-phi', 'Pion fake rate vs phi', phiBins2, phiMin, phiMax)
    fakerate_phi_denom = TH1F('FakeRate-phi-denom', 'Pion fake rate vs phi', phiBins2, phiMin, phiMax)

    for entry in file_chain:
        mcps = entry.MCParticles
        momentum = mcps[0].getMomentum()
        muids = entry.Muids
        for i in range(muids.GetEntriesFast()):
            outcome.Fill(muids[i].getOutcome())
            if muids[i].getOutcome() > 0:
                llMu = muids[i].getLogL_mu()
                llPi = muids[i].getLogL_pi()
                blayer = muids[i].getBarrelHitLayer()
                elayer = muids[i].getEndcapHitLayer()
                diffLayer = muids[i].getExtLayer() - muids[i].getHitLayer()
                ndof = muids[i].getDegreesOfFreedom()
                chisq = muids[i].getChiSquared()
                rchisq = -1.0
                if ndof > 0:
                    rchisq = chisq / ndof
                p = momentum.Mag()
                theta = momentum.Theta() * 180.0 / np.pi
                phi = momentum.Phi() * 180.0 / np.pi
                if phi < 0.0:
                    phi = phi + 360.0
                if 'muon' in file_chain.GetCurrentFile().GetName():
                    llMu_mu.Fill(llMu)
                    llPi_mu.Fill(llPi)
                    llDiff_mu.Fill(llMu - llPi)
                    barrelLayer_mu.Fill(blayer)
                    endcapLayer_mu.Fill(elayer)
                    diffLayer_mu.Fill(diffLayer)
                    ndof_mu.Fill(ndof)
                    rchisq_mu.Fill(rchisq)
                    efficiency_momentum_denom.Fill(p)
                    efficiency_theta_denom.Fill(theta)
                    efficiency_phi_denom.Fill(phi)
                    if llMu > llPi:
                        efficiency_momentum.Fill(p)
                        efficiency_theta.Fill(theta)
                        efficiency_phi.Fill(phi)
                else:
                    llMu_pi.Fill(llMu)
                    llPi_pi.Fill(llPi)
                    llDiff_pi.Fill(llMu - llPi)
                    barrelLayer_pi.Fill(blayer)
                    endcapLayer_pi.Fill(elayer)
                    diffLayer_pi.Fill(diffLayer)
                    ndof_pi.Fill(ndof)
                    rchisq_pi.Fill(rchisq)
                    fakerate_momentum_denom.Fill(p)
                    fakerate_theta_denom.Fill(theta)
                    fakerate_phi_denom.Fill(phi)
                    if llMu > llPi:
                        fakerate_momentum.Fill(p)
                        fakerate_theta.Fill(theta)
                        fakerate_phi.Fill(phi)

    for j in range(efficiency_momentum_denom.GetNbinsX()):
        num = efficiency_momentum.GetBinContent(j + 1)
        denom = efficiency_momentum_denom.GetBinContent(j + 1)
        if denom > 0:
            efficiency_momentum.SetBinContent(j + 1, num / denom)
            efficiency_momentum.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))
    for j in range(efficiency_theta_denom.GetNbinsX()):
        num = efficiency_theta.GetBinContent(j + 1)
        denom = efficiency_theta_denom.GetBinContent(j + 1)
        if denom > 0:
            efficiency_theta.SetBinContent(j + 1, num / denom)
            efficiency_theta.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))
    for j in range(efficiency_phi_denom.GetNbinsX()):
        num = efficiency_phi.GetBinContent(j + 1)
        denom = efficiency_phi_denom.GetBinContent(j + 1)
        if denom > 0:
            efficiency_phi.SetBinContent(j + 1, num / denom)
            efficiency_phi.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))
    for j in range(fakerate_momentum_denom.GetNbinsX()):
        num = fakerate_momentum.GetBinContent(j + 1)
        denom = fakerate_momentum_denom.GetBinContent(j + 1)
        if denom > 0:
            fakerate_momentum.SetBinContent(j + 1, num / denom)
            fakerate_momentum.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))
    for j in range(fakerate_theta_denom.GetNbinsX()):
        num = fakerate_theta.GetBinContent(j + 1)
        denom = fakerate_theta_denom.GetBinContent(j + 1)
        if denom > 0:
            fakerate_theta.SetBinContent(j + 1, num / denom)
            fakerate_theta.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))
    for j in range(fakerate_phi_denom.GetNbinsX()):
        num = fakerate_phi.GetBinContent(j + 1)
        denom = fakerate_phi_denom.GetBinContent(j + 1)
        if denom > 0:
            fakerate_phi.SetBinContent(j + 1, num / denom)
            fakerate_phi.SetBinError(j + 1, math.sqrt(num * (denom - num) / (denom * denom * denom)))

    # NOTE: *.Fill() must precede *.GetListOfFunctions().Add() or the latter will be discarded!
    outcome.GetListOfFunctions().Add(TNamed('Description', "0=not in KLM, 1/2=barrel/endcap stop, 3/4=barrel/endcap exit"))
    outcome.GetListOfFunctions().Add(TNamed('Check', "Peak at 3; rest ~ flat"))
    outcome.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    outcome.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    outcome.SetMinimum(0.0)

    llMu_mu.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood of muon hypothesis for true muons"))
    llMu_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at 0 with tail on negative side"))
    llMu_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llMu_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    llMu_pi.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood of muon hypothesis for true pions"))
    llMu_pi.GetListOfFunctions().Add(TNamed('Check', "Sharp double peak at ~52 with tail up to 0"))
    llMu_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llMu_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    llPi_mu.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood of pion hypothesis for true muons"))
    llPi_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at ~-42 with tail up to 0"))
    llPi_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llPi_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    llPi_pi.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood of pion hypothesis for true pions"))
    llPi_pi.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at 0 with tail on negative side"))
    llPi_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llPi_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    llDiff_mu.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood difference of muon-pion hypotheses for true muons"))
    llDiff_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at ~+42 with tail on lower side"))
    llDiff_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llDiff_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    llDiff_pi.GetListOfFunctions().Add(TNamed('Description', "Log-likelihood difference of muon-pion hypotheses for true pions"))
    llDiff_pi.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at ~-52 with tail on higher side"))
    llDiff_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    llDiff_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))

    barrelLayer_mu.GetListOfFunctions().Add(TNamed('Description', "Outermost barrel layer with matching hit for true muons"))
    barrelLayer_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at layer 14"))
    barrelLayer_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    barrelLayer_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    barrelLayer_pi.GetListOfFunctions().Add(TNamed('Description', "Outermost barrel layer with matching hit for true pions"))
    barrelLayer_pi.GetListOfFunctions().Add(TNamed('Check', "Peak at layers 0-1 with tail above that"))
    barrelLayer_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    barrelLayer_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    endcapLayer_mu.GetListOfFunctions().Add(TNamed('Description', "Outermost endcap layer with matching hit for true muons"))
    endcapLayer_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peaks at layers 11 (backward) and 13 (forward)"))
    endcapLayer_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    endcapLayer_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    endcapLayer_pi.GetListOfFunctions().Add(TNamed('Description', "Outermost endcap layer with matching hit for true pions"))
    endcapLayer_pi.GetListOfFunctions().Add(TNamed('Check', "Peak at layers 0-1 with tail above that"))
    endcapLayer_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    endcapLayer_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))

    diffLayer_mu.GetListOfFunctions().Add(TNamed('Description', "Outermost extrapolated-vs-hit layer difference for true muons"))
    diffLayer_mu.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at 0"))
    diffLayer_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    diffLayer_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    diffLayer_mu.GetXaxis().SetTitle('Last Ext Layer - Last Hit Layer')
    diffLayer_pi.GetListOfFunctions().Add(TNamed('Description', "Outermost extrapolated-vs-hit layer difference for true pions"))
    diffLayer_pi.GetListOfFunctions().Add(TNamed('Check', "Sharp peak at 15 with tail below that"))
    diffLayer_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    diffLayer_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    diffLayer_pi.GetXaxis().SetTitle('Last Ext Layer - Last Hit Layer')

    ndof_mu.GetListOfFunctions().Add(TNamed('Description', "(Number of matching-hit layers) x 2 for true muons"))
    ndof_mu.GetListOfFunctions().Add(TNamed('Check', "Even-only entries; peak at 30 with tail below"))
    ndof_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    ndof_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    ndof_mu.GetXaxis().SetTitle('#dof')
    ndof_pi.GetListOfFunctions().Add(TNamed('Description', "(Number of matching-hit layers) x 2 for true pions"))
    ndof_pi.GetListOfFunctions().Add(TNamed('Check', "Even-only entries; sharp peak at 0 with tail above"))
    ndof_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    ndof_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    ndof_pi.GetXaxis().SetTitle('#dof')

    rchisq_mu.GetListOfFunctions().Add(TNamed('Description', "Transverse-deviation chi-squared/#dof for true muons"))
    rchisq_mu.GetListOfFunctions().Add(TNamed('Check', "Narrow peak at 1"))
    rchisq_mu.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    rchisq_mu.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    rchisq_mu.GetXaxis().SetTitle('#chi^{2} / #ndof')
    rchisq_pi.GetListOfFunctions().Add(TNamed('Description', "Transverse-deviation chi-squared/#dof for true pions"))
    rchisq_pi.GetListOfFunctions().Add(TNamed('Check', "Broad peak at ~1"))
    rchisq_pi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    rchisq_pi.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.50,pvalue-error=0.10'))
    rchisq_pi.GetXaxis().SetTitle('#chi^{2} / #ndof')

    efficiency_momentum.GetListOfFunctions().Add(TNamed('Description', "Muon ID efficiency vs p for logL(mu) > logL(pi)"))
    efficiency_momentum.GetListOfFunctions().Add(TNamed('Check', "90-95% for p > 0.6 GeV/c"))
    efficiency_momentum.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    efficiency_momentum.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    efficiency_momentum.GetXaxis().SetTitle('p (GeV/#it{c})')
    efficiency_momentum.SetMinimum(0.0)
    fakerate_momentum.GetListOfFunctions().Add(TNamed('Description', "Pion fake rate vs p for logL(mu) > logL(pi)"))
    fakerate_momentum.GetListOfFunctions().Add(TNamed('Check', "Peak of 11% below 1 GeV/c, 2-6% above 1 GeV/c"))
    fakerate_momentum.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    fakerate_momentum.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    fakerate_momentum.GetXaxis().SetTitle('p (GeV/#it{c})')
    fakerate_momentum.SetMinimum(0.0)

    efficiency_theta.GetListOfFunctions().Add(TNamed('Description', "Muon ID efficiency vs theta for logL(mu) > logL(pi)"))
    efficiency_theta.GetListOfFunctions().Add(TNamed('Check', "Somewhat flat (80-100%) in KLM range (17 < theta < 145 degrees)"))
    efficiency_theta.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    efficiency_theta.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    efficiency_theta.GetXaxis().SetTitle('#theta (deg)')
    efficiency_theta.SetMinimum(0.0)
    fakerate_theta.GetListOfFunctions().Add(TNamed('Description', "Pion fake rate vs theta for logL(mu) > logL(pi)"))
    fakerate_theta.GetListOfFunctions().Add(TNamed('Check', "Roughly flat (3-7%) in KLM range (17 < theta < 145 degrees)"))
    fakerate_theta.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    fakerate_theta.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    fakerate_theta.GetXaxis().SetTitle('#theta (deg)')
    fakerate_theta.SetMinimum(0.0)

    efficiency_phi.GetListOfFunctions().Add(TNamed('Description', "Muon identification efficiency vs phi for logL(mu) > logL(pi)"))
    efficiency_phi.GetListOfFunctions().Add(TNamed('Check', "Somewhat flat distribution (85-100%)"))
    efficiency_phi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    efficiency_phi.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    efficiency_phi.GetXaxis().SetTitle('#phi (deg)')
    efficiency_phi.SetMinimum(0.0)
    fakerate_phi.GetListOfFunctions().Add(TNamed('Description', "Pion fake rate vs phi for logL(mu) > logL(pi)"))
    fakerate_phi.GetListOfFunctions().Add(TNamed('Check', "Roughly flat distribution (2-7%)"))
    fakerate_phi.GetListOfFunctions().Add(TNamed('Contact', "piilonen@vt.edu"))
    fakerate_phi.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    fakerate_phi.GetXaxis().SetTitle('#phi (deg)')
    fakerate_phi.SetMinimum(0.0)

    outcome.Write()
    llMu_mu.Write()
    llMu_pi.Write()
    llPi_mu.Write()
    llPi_pi.Write()
    llDiff_mu.Write()
    llDiff_pi.Write()
    barrelLayer_mu.Write()
    barrelLayer_pi.Write()
    endcapLayer_mu.Write()
    endcapLayer_pi.Write()
    diffLayer_mu.Write()
    diffLayer_pi.Write()
    ndof_mu.Write()
    ndof_pi.Write()
    rchisq_mu.Write()
    rchisq_pi.Write()
    efficiency_momentum.Write()
    efficiency_theta.Write()
    efficiency_phi.Write()
    fakerate_momentum.Write()
    fakerate_theta.Write()
    fakerate_phi.Write()

####################################################################
# Entry point of this script: call the main() function             #
####################################################################
if __name__ == '__main__':
    main()
