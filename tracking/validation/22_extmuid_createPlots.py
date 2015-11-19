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
from ROOT import TFile, TChain, TTree, TH1F, TH2F, TCanvas, TGraphErrors, TGraph, \
    gStyle, TNamed, TF1, TProfile
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

    print('Ext and Muid validation plots.')

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

    detectorID = TH1F('DetectorID', 'Detector ID for ExtHits', 16, -0.25, 7.75)
    file_chain.Draw('ExtHits.m_DetectorID>>DetectorID', '')
    detectorID.GetXaxis().SetTitle('Detector ID')
    detectorID.Write()

    tof = TH1F('TOF', 'Time of Flight for non-KLM ExtHits', 100, 0.0, 25.0)
    tof.GetXaxis().SetTitle('t (ns)')
    file_chain.Draw('ExtHits.m_TOF>>TOF', 'ExtHits.m_DetectorID!=7')
    tof.Write()

    tofKLM = TH1F('TOFKLM', 'Time of Flight for KLM ExtHits', 100, 0.0, 25.0)
    tofKLM.GetXaxis().SetTitle('t (ns)')
    file_chain.Draw('ExtHits.m_TOF>>TOFKLM', 'ExtHits.m_DetectorID==7')
    tofKLM.Write()

    xy = TH2F('xy', 'y vs x for non-KLM ExtHits', 100, -200.0, 200.0, 100, -200.0, 200.0)
    xy.GetXaxis().SetTitle('x (cm)')
    xy.GetYaxis().SetTitle('y (cm)')
    xy.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))

    file_chain.Draw('ExtHits.m_Position.Y():ExtHits.m_Position.X()>>xy', 'ExtHits.m_DetectorID!=7')
    xy.Write()

    xz = TH2F('xz', 'x vs z for non-KLM ExtHits', 125, -200.0, 300.0, 100, -200.0, 200.0)
    xz.GetXaxis().SetTitle('z (cm)')
    xz.GetYaxis().SetTitle('x (cm)')
    xz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('ExtHits.m_Position.X():ExtHits.m_Position.Z()>>xz', 'ExtHits.m_DetectorID!=7')
    xz.Write()

    yz = TH2F('yz', 'y vs z for non-KLM ExtHits', 125, -200.0, 300.0, 100, -200.0, 200.0)
    yz.GetXaxis().SetTitle('z (cm)')
    yz.GetYaxis().SetTitle('y (cm)')
    yz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('ExtHits.m_Position.Y():ExtHits.m_Position.Z()>>yz', 'ExtHits.m_DetectorID!=7')
    yz.Write()

    xyKLM = TH2F('xyKLM', 'y vs x for KLM ExtHits', 140, -350.0, 350.0, 140, -350.0, 350.0)
    xyKLM.GetXaxis().SetTitle('x (cm)')
    xyKLM.GetYaxis().SetTitle('y (cm)')
    xyKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('ExtHits.m_Position.Y():ExtHits.m_Position.X()>>xyKLM', 'ExtHits.m_DetectorID==7')
    xyKLM.Write()

    xzKLM = TH2F('xzKLM', 'x vs z for KLM ExtHits', 140, -300.0, 400.0, 140, -350.0, 350.0)
    xzKLM.GetXaxis().SetTitle('z (cm)')
    xzKLM.GetYaxis().SetTitle('x (cm)')
    xzKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('ExtHits.m_Position.X():ExtHits.m_Position.Z()>>xzKLM', 'ExtHits.m_DetectorID==7')
    xzKLM.Write()

    yzKLM = TH2F('yzKLM', 'y vs z for KLM ExtHits', 140, -300.0, 400.0, 140, -350.0, 350.0)
    yzKLM.GetXaxis().SetTitle('z (cm)')
    yzKLM.GetYaxis().SetTitle('y (cm)')
    yzKLM.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('ExtHits.m_Position.Y():ExtHits.m_Position.Z()>>yzKLM', 'ExtHits.m_DetectorID==7')
    yzKLM.Write()


def draw_likelihoods(file_chain):
    """
    Draw the Muid likelihood-based distributions.
    """

    outcome = TH1F('Outcome', 'Outcome', 10, -0.25, 4.75)
    outcome.SetMinimum(0.0)

    llBins = 1000
    llMin = -50.0
    llMax = +50.0
    llMu_mu = TH1F('LLmu-muons', 'LL(mu) for true muons', llBins, llMin, llMax)
    llMu_pi = TH1F('LLmu-pions', 'LL(mu) for true pions', llBins, llMin, llMax)
    llPi_mu = TH1F('LLpi-muons', 'LL(pi) for true muons', llBins, llMin, llMax)
    llPi_pi = TH1F('LLpi-pions', 'LL(pi) for true pions', llBins, llMin, llMax)
    llDiff_mu = TH1F('LLdiff-muons', 'LL(mu)-LL(pi) for true muons', llBins, llMin, llMax)
    llDiff_pi = TH1F('LLdiff-pions', 'LL(mu)-LL(pi) for true pions', llBins, llMin, llMax)

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
    diffLayer_mu = TH1F('DiffLayer-muons', 'Layer difference for true muons', diffLayerBins, diffLayerMin, diffLayerMax)
    diffLayer_pi = TH1F('DiffLayer-pions', 'Layer difference for true pions', diffLayerBins, diffLayerMin, diffLayerMax)
    diffLayer_mu.GetXaxis().SetTitle('Last Ext Layer - Last Hit Layer')
    diffLayer_pi.GetXaxis().SetTitle('Last Ext Layer - Last Hit Layer')

    ndofBins = 30
    ndofMin = -0.5
    ndofMax = 29.5
    ndof_mu = TH1F('Ndof-muons', 'Degrees of freedom for true muons', ndofBins, ndofMin, ndofMax)
    ndof_pi = TH1F('Ndof-pions', 'Degrees of freedom for true pions', ndofBins, ndofMin, ndofMax)
    ndof_mu.GetXaxis().SetTitle('#dof')
    ndof_pi.GetXaxis().SetTitle('#dof')

    rchisqBins = 100
    rchisqMin = 0.0
    rchisqMax = 10.0
    rchisq_mu = TH1F('rchisq-muons', 'Reduced chi-squared for true muons', rchisqBins, rchisqMin, rchisqMax)
    rchisq_pi = TH1F('rchisq-pions', 'Reduced chi-squared for true pions', rchisqBins, rchisqMin, rchisqMax)
    rchisq_mu.GetXaxis().SetTitle('#chi_{r}^{2}')
    rchisq_pi.GetXaxis().SetTitle('#chi_{r}^{2}')

    momentumBins = 50
    momentumMin = 0.0
    momentumMax = 5.0
    efficiency_momentum = TH1F('Eff-momentum', 'Muon efficiency vs momentum', momentumBins, momentumMin, momentumMax)
    efficiency_momentum_denom = TH1F('Eff-momentum-denom', 'Muon efficiency vs momentum', momentumBins, momentumMin, momentumMax)
    fakerate_momentum = TH1F('FakeRate-momentum', 'Pion fake rate vs momentum', momentumBins, momentumMin, momentumMax)
    fakerate_momentum_denom = TH1F('FakeRate-momentum-denom', 'Pion fake rate vs momentum', momentumBins, momentumMin, momentumMax)
    efficiency_momentum.GetXaxis().SetTitle('p (GeV/#it{c})')
    fakerate_momentum.GetXaxis().SetTitle('p (GeV/#it{c})')
    efficiency_momentum.SetMinimum(0.0)
    fakerate_momentum.SetMinimum(0.0)

    thetaBins = 70
    thetaMin = 10.0
    thetaMax = 150.0
    efficiency_theta = TH1F('Eff-theta', 'Muon efficiency vs theta', thetaBins, thetaMin, thetaMax)
    efficiency_theta_denom = TH1F('Eff-theta-denom', 'Muon efficiency vs theta', thetaBins, thetaMin, thetaMax)
    fakerate_theta = TH1F('FakeRate-theta', 'Pion fake rate vs theta', thetaBins, thetaMin, thetaMax)
    fakerate_theta_denom = TH1F('FakeRate-theta-denom', 'Pion fake rate vs theta', thetaBins, thetaMin, thetaMax)
    efficiency_theta.GetXaxis().SetTitle('#theta (deg)')
    fakerate_theta.GetXaxis().SetTitle('#theta (deg)')
    efficiency_theta.SetMinimum(0.0)
    fakerate_theta.SetMinimum(0.0)

    phiBins = 72
    phiMin = 0.0
    phiMax = 360.0
    efficiency_phi = TH1F('Eff-phi', 'Muon efficiency vs phi', phiBins, phiMin, phiMax)
    efficiency_phi_denom = TH1F('Eff-phi-denom', 'Muon efficiency vs phi', phiBins, phiMin, phiMax)
    fakerate_phi = TH1F('FakeRate-phi', 'Pion fake rate vs phi', phiBins, phiMin, phiMax)
    fakerate_phi_denom = TH1F('FakeRate-phi-denom', 'Pion fake rate vs phi', phiBins, phiMin, phiMax)
    efficiency_phi.GetXaxis().SetTitle('#phi (deg)')
    fakerate_phi.GetXaxis().SetTitle('#phi (deg)')
    efficiency_phi.SetMinimum(0.0)
    fakerate_phi.SetMinimum(0.0)

    for entry in file_chain:
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
                momentum = muids[i].getMomentum()
                p = momentum.Mag() / 1000.0
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
    efficiency_momentum.Divide(efficiency_momentum_denom)
    efficiency_theta.Divide(efficiency_theta_denom)
    efficiency_phi.Divide(efficiency_phi_denom)
    fakerate_momentum.Divide(fakerate_momentum_denom)
    fakerate_theta.Divide(fakerate_theta_denom)
    fakerate_phi.Divide(fakerate_phi_denom)
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
