#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>Belle_pi+_validation.root</output>
  <output>Belle_mu+_validation.root</output>
  <output>Belle_e+_validation.root</output>
  <output>Belle_K+_validation.root</output>
  <output>Belle_gamma_validation.root</output>
  <output>Belle_K_S0_validation.root</output>
  <output>Belle_pi0_validation.root</output>
  <output>MC6x1_pi+_validation.root</output>
  <output>MC6x1_mu+_validation.root</output>
  <output>MC6x1_e+_validation.root</output>
  <output>MC6x1_K+_validation.root</output>
  <output>MC6x1_gamma_validation.root</output>
  <output>MC6x1_K_S0_validation.root</output>
  <output>MC6x1_K_L0_validation.root</output>
  <output>MC6x1_pi0_validation.root</output>
  <output>MC6x0_pi+_validation.root</output>
  <output>MC6x0_mu+_validation.root</output>
  <output>MC6x0_e+_validation.root</output>
  <output>MC6x0_K+_validation.root</output>
  <output>MC6x0_gamma_validation.root</output>
  <output>MC6x0_K_S0_validation.root</output>
  <output>MC6x0_K_L0_validation.root</output>
  <output>MC6x0_pi0_validation.root</output>
  <output>MC5x0_pi+_validation.root</output>
  <output>MC5x0_mu+_validation.root</output>
  <output>MC5x0_e+_validation.root</output>
  <output>MC5x0_K+_validation.root</output>
  <output>MC5x0_gamma_validation.root</output>
  <output>MC5x0_K_S0_validation.root</output>
  <output>MC5x0_K_L0_validation.root</output>
  <output>MC5x0_pi0_validation.root</output>
  <output>MC5x1_pi+_validation.root</output>
  <output>MC5x1_mu+_validation.root</output>
  <output>MC5x1_e+_validation.root</output>
  <output>MC5x1_K+_validation.root</output>
  <output>MC5x1_gamma_validation.root</output>
  <output>MC5x1_K_S0_validation.root</output>
  <output>MC5x1_K_L0_validation.root</output>
  <output>MC5x1_pi0_validation.root</output>
  <contact>Thomas Keck; thomas.keck2@kit.edu</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
import pdg

import ROOT
from ROOT import Belle2

import variables as v
v.variables.addAlias('uniquePrimarySignal', 'extraInfo(uniquePrimarySignal)')

import os
import sys

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    sys.exit(0)

mc_files = [
    # both Belle files (exp 65) contain #10965 events (before BHadron cut)
    ('Belle', ['mdstBelle1_exp65_mixed.root', 'mdstBelle1_exp65_charged.root']),
    ('MC6x1', ['mdst6_BBx1_mixed.root', 'mdst6_BBx1_charged.root']),
    ('MC6x0', ['mdst6_BBx0_mixed.root', 'mdst6_BBx0_charged.root']),
    ('MC5x1', ['mdst5_BBx1_mixed.root', 'mdst5_BBx1_charged.root']),
    ('MC5x0', ['mdst5_BBx0_mixed.root', 'mdst5_BBx0_charged.root']),
]


fsps = ['pi+', 'mu+', 'e+', 'K+', 'gamma', 'K_S0', 'K_L0', 'pi0']


def NMcName(fsp):
    if fsp.endswith('+'):
        return 'NumberOfMCParticles' + fsp[:-1]
    return 'NumberOfMCParticles' + fsp


for fsp in fsps:
    v.variables.addAlias(NMcName(fsp), 'NumberOfMCParticlesInEvent({})'.format(abs(pdg.from_name(fsp))))
v.variables.addAlias('pt_mc', 'matchedMC(pt)')
v.variables.addAlias('E_mc', 'matchedMC(E)')

for mc, files in mc_files:

    files = [os.path.join(str(os.environ['BELLE2_VALIDATION_DATA_DIR']), 'analysis/', f) for f in files]
    path = create_path()

    if mc == 'Belle':
        import b2biiConversion
        b2biiConversion.setupB2BIIDatabase(True)
        b2biiConversion.convertBelleMdstToBelleIIMdst(inputBelleMDSTFile=files, applyHadronBJSkim=True, path=path)
        copyList('gamma', 'gamma:mdst', path=path)
        copyList('K_S0', 'K_S0:mdst', path=path)
        copyList('pi0', 'pi0:mdst', path=path)
    elif mc.startswith('MC6'):
        inputMdstList('default', files, path=path)
    elif mc.startswith('MC5'):
        inputMdstList('MC5', files, path=path)
    else:
        raise RuntimeError("Unkown MC {}".format(mc))

    fillParticleLists([(fsp, '') for fsp in fsps[:4]], path=path)

    if mc.startswith('MC'):
        fillParticleLists([(fsp, '') for fsp in fsps[4:7]], path=path)
        reconstructDecay('pi0 -> gamma gamma', '0.08 < M < 0.180', path=path)

    fillParticleListsFromMC([(fsp + ':mc', 'isPrimarySignal == 1') for fsp in fsps], path=path)

    variablesToNTuple('', [NMcName(fsp) for fsp in fsps], filename='event.root', path=path)

    for fsp in fsps:
        if mc == 'Belle' and fsp == 'K_L0':
            continue
        matchMCTruth(fsp, path=path)
        path.add_module('TagUniqueSignal', particleList=fsp, target='isPrimarySignal', extraInfoName='uniquePrimarySignal')
        path.add_module('TagUniqueSignal', particleList=fsp + ':mc', target='isPrimarySignal', extraInfoName='uniquePrimarySignal')

        outputfile = '../' + mc + '_' + fsp + '.root'
        outputfile_mc = '../' + mc + '_' + fsp + '_mc.root'
        common_variables = [
            'E',
            'pt_mc',
            'E_mc',
            'pt',
            'px',
            'py',
            'pz',
            'dr',
            'dz',
            'mcErrors',
            'mcParticleStatus',
            'mcSecPhysProc',
            'mcPrimary',
            'mcISR',
            'mcFSR',
            'mcPhotos',
            'isPrimarySignal',
            'uniquePrimarySignal']
        if fsp == 'gamma':
            variablesToNTuple('gamma', ['goodBelleGamma' if mc == 'Belle' else 'goodGamma', 'clusterReg',
                                        'clusterTiming'] + common_variables, filename=outputfile, path=path)
            variablesToNTuple('gamma:mc', ['goodBelleGamma' if mc == 'Belle' else 'goodGamma',
                                           'clusterReg', 'clusterTiming'] + common_variables, filename=outputfile_mc, path=path)
        else:
            variablesToNTuple(fsp, common_variables, filename=outputfile, path=path)
            variablesToNTuple(fsp + ':mc', common_variables, filename=outputfile_mc, path=path)

    process(path)

    for fsp in fsps:
        if mc == 'Belle' and fsp == 'K_L0':
            continue
        filename = '../' + mc + '_' + fsp + '.root'
        filename_mc = '../' + mc + '_' + fsp + '_mc.root'
        tfile = ROOT.TFile.Open(filename)
        tfile_mc = ROOT.TFile.Open(filename_mc)
        ttree = tfile.Get("variables")
        ttree_mc = tfile_mc.Get("variables")
        histname_all = mc + '_' + fsp + '_all'
        histname_eff = mc + '_' + fsp + '_eff'
        histname_sig = mc + '_' + fsp + '_sig'
        histname_pur = mc + '_' + fsp + '_pur'
        histname_mc = mc + '_' + fsp + '_mc'

        tfile_validation = ROOT.TFile.Open(mc + '_' + fsp + '_validation.root', "RECREATE")

        if fsp == 'gamma':
            thist_eff = ROOT.TH1F(histname_eff, fsp + " efficiency on " + mc, 200, 0.0, 5.0)
            thist_sig = ROOT.TH1F(histname_sig, fsp + " number on " + mc, 200, 0.0, 5.0)
            thist_pur = ROOT.TH1F(histname_pur, fsp + " purity on " + mc, 200, 0.0, 5.0)
            thist_all = ROOT.TH1F(histname_all, fsp + " all on " + mc, 200, 0.0, 5.0)
            thist_mc = ROOT.TH1F(histname_mc, fsp + " mc on " + mc, 200, 0.0, 5.0)
            ttree.Project(histname_all, "E_mc")
            ttree.Project(histname_sig, "E_mc", "uniquePrimarySignal == 1")
            ttree.Project(histname_eff, "E_mc", "uniquePrimarySignal == 1")
            ttree.Project(histname_pur, "E_mc", "uniquePrimarySignal == 1")
            ttree_mc.Project(histname_mc, "E_mc", "uniquePrimarySignal == 1")
            thist_sig.SetXTitle("E")
            thist_eff.SetXTitle("E")
            thist_pur.SetXTitle("E")
            thist_mc.SetXTitle("E")
        else:
            thist_sig = ROOT.TH1F(histname_sig, fsp + " number on " + mc, 200, 0.0, 5.0)
            thist_eff = ROOT.TH1F(histname_eff, fsp + " efficiency on " + mc, 200, 0.0, 5.0)
            thist_pur = ROOT.TH1F(histname_pur, fsp + " purity on " + mc, 200, 0.0, 5.0)
            thist_all = ROOT.TH1F(histname_all, fsp + " all on " + mc, 200, 0.0, 5.0)
            thist_mc = ROOT.TH1F(histname_mc, fsp + " mc on " + mc, 200, 0.0, 5.0)
            ttree.Project(histname_all, "pt_mc")
            ttree.Project(histname_eff, "pt_mc", "uniquePrimarySignal == 1")
            ttree.Project(histname_sig, "pt_mc", "uniquePrimarySignal == 1")
            ttree.Project(histname_pur, "pt_mc", "uniquePrimarySignal == 1")
            ttree_mc.Project(histname_mc, "pt_mc", "uniquePrimarySignal == 1")
            thist_eff.SetXTitle("pt")
            thist_sig.SetXTitle("pt")
            thist_pur.SetXTitle("pt")
            thist_mc.SetXTitle("pt")

        thist_eff.SetYTitle("Efficiency")
        thist_pur.SetYTitle("Purity")
        thist_mc.SetYTitle("Entries")
        thist_sig.SetYTitle("Entries")
        thist_eff.Divide(thist_mc)
        thist_pur.Divide(thist_all)
        thist_sig.GetListOfFunctions().Add(ROOT.TNamed("Desription",
                                                       "Number of correctly reconstructed unique primary " + fsp + " on " + mc))
        thist_sig.GetListOfFunctions().Add(ROOT.TNamed("Check", ""))
        thist_eff.GetListOfFunctions().Add(ROOT.TNamed("Desription",
                                                       "Efficiency correctly reconstructed unique primary " + fsp + " on " + mc))
        thist_eff.GetListOfFunctions().Add(ROOT.TNamed("Check", ""))
        thist_pur.GetListOfFunctions().Add(ROOT.TNamed("Desription", "Purity of reconstructed " + fsp + " on " + mc))
        thist_pur.GetListOfFunctions().Add(ROOT.TNamed("Check", ""))
        thist_mc.GetListOfFunctions().Add(ROOT.TNamed("Desription", "Number of primary MC Particles of type " + fsp + " on " + mc))
        thist_mc.GetListOfFunctions().Add(ROOT.TNamed("Check", ""))

        tfile_validation.Write(histname_sig)
        tfile_validation.Write(histname_eff)
        tfile_validation.Write(histname_pur)
        tfile_validation.Write(histname_mc)

        eff = thist_sig.GetEntries() / thist_mc.GetEntries()
        pur = thist_sig.GetEntries() / thist_all.GetEntries()
        print(fsp, mc, eff, pur)
        ntuplename = mc + '_' + fsp + '_ntuple'
        ntuple = ROOT.TNtuple(ntuplename, "Statistics", "eff:pur")
        ntuple.Fill(eff, pur)
        ntuple.SetAlias("Description", "Efficiency and Purity of correctly reconstructed unique primary " + fsp + " on " + mc)
        ntuple.SetAlias("Check", "")
        tfile_validation.Write(ntuplename)
