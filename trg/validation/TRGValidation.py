#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>TRGValidationGen.root</input>
  <output>TRGValidation.root</output>
  <contact>yinjh2012@korea.ac.kr</contact>
  <description>makes validation plots for TRG</description>
</header>
"""


import basf2
import ROOT
from ROOT import Belle2, gROOT, TH1F, TFile, TNamed
from math import pi as PI

Fac = 180.0 / PI


class MakePlots(basf2.Module):
    '''
    Make validation histograms for trg ecl/cdc/klm
    '''

    def set_descr(self, histogram, description, check):
        '''
        Sets description, check and contact to validation histogram.
        :param h validation histogram
        :param Descr description text
        '''

        descr = TNamed('Description', description)
        histogram.GetListOfFunctions().Add(descr)
        Check = TNamed('Check', check)
        histogram.GetListOfFunctions().Add(Check)
        contact = TNamed('Contact', 'yinjh2012@korea.ac.kr')
        histogram.GetListOfFunctions().Add(contact)
        Meta = TNamed("MetaOptions", "shifter")
        histogram.GetListOfFunctions().Add(Meta)

    def set_style(self, histogram, xtitle, ytitle):
        '''
        Sets x-y titles, and sets histogram style.
        :param xtitle X-axis title
        :param xtitle Y-axis title
        '''

        histogram.GetXaxis().SetTitle(xtitle)
        histogram.GetYaxis().SetTitle(ytitle)
        histogram.GetYaxis().SetTitleOffset(1.4)
        histogram.GetXaxis().SetTitleSize(0.045)
        histogram.GetYaxis().SetLabelSize(0.020)
        histogram.SetLineColor(ROOT.kBlack)

    def initialize(self):

        self.tfile = TFile('TRGValidation.root', 'recreate')

        m_dbinput = Belle2.PyDBObj('TRGGDLDBInputBits')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')
        n_inbit = m_dbinput.getninbit()
        n_outbit = m_dbftdl.getnoutbit()

        mc_px = 'MCParticles.m_momentum_x'
        mc_py = 'MCParticles.m_momentum_y'
        mc_pz = 'MCParticles.m_momentum_z'
        trk2d_omega = 'TRGCDC2DFinderTracks.m_omega'
        trk2d_phi = 'TRGCDC2DFinderTracks.m_phi0'

        ROOT.gStyle.SetOptStat(ROOT.kFALSE)

        #: validation histogram
        self.hist_inbit = TH1F('hin', 'trigger input bits', 320, 0, 320)
        self.hist_inbit.GetXaxis().SetRangeUser(0, n_inbit)
        self.hist_inbit.GetXaxis().SetLabelSize(0.02)

        #: validation histogram
        self.hist_outbit = TH1F('hout', 'trigger output bits', 320, 0, 320)
        self.hist_outbit.GetXaxis().SetRangeUser(0, n_outbit)
        self.hist_outbit.GetXaxis().SetLabelSize(0.02)

        for i in range(320):
            inbitname = m_dbinput.getinbitname(i)
            outbitname = m_dbftdl.getoutbitname(i)
            self.hist_inbit.GetXaxis().SetBinLabel(self.hist_inbit.GetXaxis().FindBin(i + 0.5), inbitname)
            self.hist_outbit.GetXaxis().SetBinLabel(self.hist_outbit.GetXaxis().FindBin(i + 0.5), outbitname)

        #: validation histogram
        self.h_E_ECL = TH1F("h_E_ECL", "ECL cluster energy [5 MeV]", 200, 0, 10)
        self.set_descr(self.h_E_ECL, 'TRG ECL cluster energy', '')
        self.set_style(self.h_E_ECL, "ECL cluster energy (GeV)", "Events/(50 MeV)")

        #: validation histogram
        self.h_Esum_ECL = TH1F("h_Esum_ECL", "sum of ECL cluster energy [5 MeV]", 200, 0, 10)
        self.set_descr(self.h_Esum_ECL, "sum of TRG ECL cluster energy", "")
        self.set_style(self.h_Esum_ECL, "sum of ECL cluster energy (GeV)", "Events/(50 MeV)")

        #: validation histogram
        self.h_theta_ECL = TH1F("h_theta_ECL", "TRG ECL cluster #theta [1.4 degrees]", 128, 0, 180)
        self.set_descr(self.h_theta_ECL, "TRG ECL cluster polar angle", "unform in barrel")
        self.set_style(self.h_theta_ECL, "TRG ECL cluster polar angle (degree)", "Events/(1.4 degree)")

        #: validation histogram
        self.h_thetaID_ECL = TH1F("h_thetaID_ECL", "ECL cluster TC ID", 610, 0, 610)
        self.set_descr(self.h_thetaID_ECL, 'TRG ECL cluster theta ID', 'unform in barrel')
        self.set_style(self.h_thetaID_ECL, 'ECL cluster TC ID', 'Events/(1)')

        #: validation histogram
        self.h_phi_ECL = TH1F("h_phi_ECL", "TRG ECL cluster phi [2.0 degrees]", 180, -180, 180)
        self.set_descr(self.h_phi_ECL, 'TRG ECL cluster phi distribution', 'distribute unformly')
        self.set_style(self.h_phi_ECL, "ECL cluster #phi (degree) ", "Events/(2.0 degrees)")

        #: validation histogram
        self.h_sector_BKLM = TH1F("h_sector_BKLM", "BKLM TRG hit sector", 10, 0, 10)
        self.set_descr(self.h_sector_BKLM, '# of BKLM TRG sector', 'peak at 0')
        self.set_style(self.h_sector_BKLM, "# of BKLM TRG sector", 'Events/(1)')

        #: validation histogram
        self.h_sector_EKLM = TH1F("h_sector_EKLM", "EKLM TRG hit sector", 10, 0, 10)
        self.set_descr(self.h_sector_EKLM, "# of EKLM TRG sector", 'peak at 0')
        self.set_style(self.h_sector_EKLM, "# of EKLM TRG sector", 'Events/(1)')

        mc = "abs(MCParticles.m_pdg)==11&&MCParticles.m_status==11"
        tree = ROOT.TChain('tree')
        tree.Add('../TRGValidationGen.root')

        #: validation histogram
        self.d_w = TH1F("d_w", "#Deltaw of CDC 2D finder, w = 0.00449/p_{t}", 50, -0.02, 0.02)
        self.d_w_2 = TH1F("d_w_2", "d_w_2", 50, -0.02, 0.02)
        tree.Draw("({0} - 0.00449)/sqrt({1}*{1} + {2}*{2})>> d_w".format(trk2d_omega, mc_px, mc_py),
                  "MCParticles.m_pdg<0&&" + mc)
        tree.Draw("({0} + 0.00449)/sqrt({1}*{1} + {2}*{2}) >> d_w_2".format(trk2d_omega, mc_px, mc_py),
                  "MCParticles.m_pdg>0 && " + mc)
        self.d_w.Add(self.d_w_2)
        self.set_descr(self.d_w, 'Comparison on w (=0.00449/pt) of a track between CDC 2D finder output and MC.',
                                 'A clear peak at 0 with tail.')
        self.set_style(self.d_w, "#Deltaw", "Events/(0.08)")

        #: validation histogram
        self.d_phi = TH1F("d_phi", "#Delta#phi of CDC 2D finder", 50, -0.5, 0.5)
        tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>>d_phi",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  "fabs(TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x))<3.1415936&&" + mc)

        self.d_phi_2 = TH1F("d_phi_2", "d_phi_2", 50, -0.5, 0.5)
        tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)-3.1415936>>d_phi_2",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>=3.1415936 &&" + mc)

        self.d_phi_3 = TH1F("d_phi_3", "d_phi_3", 50, -0.5, 0.5)
        tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)+3.1415936>>d_phi_2",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)<=-3.1415936&&" + mc)

        self.d_phi.Add(self.d_phi_2)
        self.d_phi.Add(self.d_phi_3)
        self.set_descr(self.d_phi, "Comparison on phi_i of a track between CDC 2D finder output and MC.",
                                   "A Gaussian peak at 0.")
        self.set_style(self.d_phi, "#Delta#phi [rad]", "Events/(0.02 rad)")

        #: validation histogram
        self.d_z0_3d = TH1F("d_z0_3d", "#Deltaz0 of CDC 3D fitter", 60, -30, 30)
        tree.Draw("TRGCDC3DFitterTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_3d", mc)
        self.set_descr(self.d_z0_3d, "Comparison on z0 of a track between CDC 2D fitter output and MC.",
                                     "A Gaussian peak at 0 with small tail.")
        self.set_style(self.d_z0_3d, "#Deltaz0 [cm]", "Events/(1 cm)")

        #: validation histogram
        self.d_z0_nn = TH1F("d_z0_nn", "#Deltaz0 of CDC Neuro", 60, -30, 30)
        tree.Draw("TRGCDCNeuroTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_nn", mc)
        self.set_descr(self.d_z0_nn, "Comparison on z0 of a track between CDC Neuro output and MC.",
                                     "A Gaussian peak at 0 with small tail.")
        self.set_style(self.d_z0_nn, "#Deltaz0 [cm]", "Events/(1 cm)")

        #: validation histogram
        self.d_E_ECL = TH1F("d_E_ECL", "#DeltaE of ECL clustering", 100, -6, 6)
        tree.Draw("TRGECLClusters.m_edep-MCParticles.m_energy>>d_E_ECL", mc)
        self.set_descr(self.d_E_ECL, "Comparison on deposit energy between ECL cluster output and MC.",
                                     "A peak around -0.5 ~ 0 with a tail toward -6.")
        self.set_style(self.d_E_ECL, "#DeltaE [GeV]", "Events/(0.12 GeV)")

    def beginRun(self):
        self.hist_inbit.Reset()
        self.hist_outbit.Reset()
        self.h_Esum_ECL.Reset()
        self.h_E_ECL.Reset()
        self.h_theta_ECL.Reset()
        self.h_thetaID_ECL.Reset()
        self.h_phi_ECL.Reset()

    def event(self):

        event_meta = Belle2.PyStoreObj('EventMetaData')
        trg_summary = Belle2.PyStoreObj('TRGSummary')
        clusters = Belle2.PyStoreArray('TRGECLClusters')
        klmSummary = Belle2.PyStoreObj('KLMTrgSummary')

        for i in range(320):
            if(trg_summary.testInput(i)):
                self.hist_inbit.Fill(i + 0.5)
            if(trg_summary.testFtdl(i)):
                self.hist_outbit.Fill(i + 0.5)

        etot = 0
        for cluster in clusters:
            x = cluster.getPositionX()
            y = cluster.getPositionY()
            z = cluster.getPositionZ()
            e = cluster.getEnergyDep()
            self.h_E_ECL.Fill(e)
            etot += e
            vec = ROOT.TVector3(x, y, z)
            self.h_theta_ECL.Fill(vec.Theta() * Fac)
            self.h_thetaID_ECL.Fill(cluster.getMaxTCId())
            self.h_phi_ECL.Fill(vec.Phi() * Fac)

        if etot > 0:
            self.h_Esum_ECL.Fill(etot)

        self.h_sector_BKLM.Fill(klmSummary.getBKLM_n_trg_sectors())
        self.h_sector_EKLM.Fill(klmSummary.getEKLM_n_trg_sectors())

    def endRun(self):
        print('end')

    def terminate(self):

        self.hist_inbit.Write()
        self.hist_outbit.Write()
        self.h_Esum_ECL.Write()
        self.h_E_ECL.Write()
        self.h_theta_ECL.Write()
        self.h_thetaID_ECL.Write()
        self.h_phi_ECL.Write()
        self.h_sector_BKLM.Write()
        self.h_sector_EKLM.Write()
        self.d_w.Write()
        self.d_phi.Write()
        self.d_z0_3d.Write()
        self.d_z0_nn.Write()
        self.d_E_ECL.Write()
        self.tfile.Close()


# Create path
main = basf2.create_path()

# INput
root_input = basf2.register_module('RootInput')
root_input.param('inputFileName', '../TRGValidationGen.root')
# root_input.param('branchNames', ['EventMetaData', 'TRGECLClusters', 'TRGSummary', 'KLMTrgSummary'])
main.add_module(root_input)

# Make plots
main.add_module(MakePlots())

basf2.process(main)
