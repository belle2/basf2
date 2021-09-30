#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2
from basf2 import *

import ROOT
from ROOT import Belle2, gROOT
import time
from array import array
import numpy as np
import rawdata
import tracking
import reconstruction
from cdc.cr import *
from tracking import add_cdc_track_finding
from reconstruction import *
from rawdata import add_unpackers
from modularAnalysis import applyEventCuts

PI = 3.1415926
Fac = 180.0 / PI


class trg_read(basf2.Module):

    root_file = ROOT.TFile('TRGValidation.root', 'recreate')

    ROOT.gStyle.SetOptStat(ROOT.kFALSE)

    hist_inbit = ROOT.TH1F('hin', 'trigger input bits', 320, 0, 320)
    hist_outbit = ROOT.TH1F('hout', 'trigger output bits', 320, 0, 320)

    h_E_ECL = ROOT.TH1F("h_E_ECL", "ECL cluster energy [5 MeV]", 2048, 0, 10.24)
    h_E_ECL.GetXaxis().SetTitle("ECL cluster energy [5 MeV]")
    h_Esum_ECL = ROOT.TH1F("h_Esum_ECL", "sum of ECL cluster energy [5 MeV]", 2048, 0, 10.24)
    h_Esum_ECL.GetXaxis().SetTitle("sum of ECL cluster energy [5 MeV]")
    h_theta_ECL = ROOT.TH1F("h_theta_ECL", "ECL cluster #theta [1.4 degrees]", 128, 0, 180)
    h_theta_ECL.GetXaxis().SetTitle("ECL cluster #theta [1.4 degrees]")
    h_thetaID_ECL = ROOT.TH1F("h_thetaID_ECL", "ECL cluster TC ID", 610, 0, 610)
    h_thetaID_ECL.GetXaxis().SetTitle("ECL cluster TC ID")
    h_phi_ECL = ROOT.TH1F("h_phi_ECL", "ECL cluster phi [1.4 degrees]", 256, -180, 180)
    h_phi_ECL.GetXaxis().SetTitle("ECL cluster #phi [1.4 degrees]")

    h_sector_BKLM = ROOT.TH1F("h_sector_BKLM", "BKLM TRG hit sector", 10, 0, 10)
    h_sector_BKLM.GetXaxis().SetTitle("# of BKLM TRG sector")
    h_sector_EKLM = ROOT.TH1F("h_sector_EKLM", "EKLM TRG hit sector", 10, 0, 10)
    h_sector_EKLM.GetXaxis().SetTitle("# of EKLM TRG sector")

    mc = "abs(MCParticles.m_pdg)==11&&MCParticles.m_status==11"
    tree = ROOT.TChain('tree')
    tree.Add('../TRGValidationGen.root')

    d_w = ROOT.TH1F("d_w", "#Deltaw of CDC 2D finder, w = 0.00449/p_{t}", 50, -0.02, 0.02)
    d_w_2 = ROOT.TH1F("d_w_2", "d_w_2", 50, -0.02, 0.02)
    d_phi = ROOT.TH1F("d_phi", "#Delta#phi of CDC 2D finder", 50, -0.5, 0.5)
    d_phi_2 = ROOT.TH1F("d_phi_2", "d_phi_2", 50, -0.5, 0.5)
    d_phi_3 = ROOT.TH1F("d_phi_3", "d_phi_3", 50, -0.5, 0.5)
    d_z0_3d = ROOT.TH1F("d_z0_3d", "#Deltaz0 of CDC 3D fitter", 60, -30, 30)
    d_z0_nn = ROOT.TH1F("d_z0_nn", "#Deltaz0 of CDC Neuro", 60, -30, 30)
    d_E_ECL = ROOT.TH1F("d_E_ECL", "#DeltaE of ECL clustering", 50, -6, 0)

    mc_px = 'MCParticles.m_momentum_x'
    mc_py = 'MCParticles.m_momentum_y'
    mc_pz = 'MCParticles.m_momentum_z'
    trk2d_omega = 'TRGCDC2DFinderTracks.m_omega'
    trk2d_phi = 'TRGCDC2DFinderTracks.m_phi0'

    tree.Draw("({0} - 0.00449)/sqrt({1}*{1} + {2}*{2})>> d_w".format(trk2d_omega, mc_px, mc_py),
              "MCParticles.m_pdg<0&&" + mc)

    tree.Draw("({0} + 0.00449)/sqrt({1}*{1} + {2}*{2}) >> d_w_2".format(trk2d_omega, mc_px, mc_py),
              "MCParticles.m_pdg>0 && " + mc)
    tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>>d_phi",
              "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
              "fabs(TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x))<3.1415936&&" + mc)
    tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)-3.1415936>>d_phi_2",
              "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
              "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>=3.1415936 &&" + mc)
    tree.Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)+3.1415936>>d_phi_2",
              "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
              "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)<=-3.1415936&&" + mc)
    tree.Draw("TRGCDC3DFitterTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_3d", mc)
    tree.Draw("TRGCDCNeuroTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_nn", mc)
    tree.Draw("TRGECLClusters.m_edep-MCParticles.m_energy>>d_E_ECL", mc)

    d_w.Add(d_w_2)
    d_phi.Add(d_phi_2)
    d_phi.Add(d_phi_3)

    d_w.SetLineColor(ROOT.kBlack)
    d_w.SetLineWidth(3)
    d_w.GetListOfFunctions().Add(
        ROOT.TNamed("Description", "Comparison on w (=0.00449/pt) of a track between CDC 2D finder output and MC."))
    d_w.GetListOfFunctions().Add(ROOT.TNamed("Contact", "ytlai@post.kek.jp"))
    d_w.GetListOfFunctions().Add(ROOT.TNamed("Check", "A clear peak at 0 with tail."))
    d_w.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", "shifter"))

    d_w.GetXaxis().SetTitle("#Deltaw")
    d_w.GetYaxis().SetTitle("Events/(0.08)")
    d_w.GetYaxis().SetTitleOffset(1.4)
    d_w.GetXaxis().SetTitleSize(0.045)
    d_w.GetYaxis().SetLabelSize(0.020)

    d_phi.SetLineColor(ROOT.kBlack)
    d_phi.SetLineWidth(3)
    d_phi.GetListOfFunctions().Add(
        ROOT.TNamed("Description", "Comparison on phi_i of a track between CDC 2D finder output and MC."))
    d_phi.GetListOfFunctions().Add(ROOT.TNamed("Contact", "ytlai@post.kek.jp"))
    d_phi.GetListOfFunctions().Add(ROOT.TNamed("Check", "A Gaussian peak at 0."))
    d_phi.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", "shifter"))

    d_phi.GetXaxis().SetTitle("#Delta#phi [rad]")
    d_phi.GetYaxis().SetTitle("Events/(0.02 rad)")
    d_phi.GetYaxis().SetTitleOffset(1.4)
    d_phi.GetXaxis().SetTitleSize(0.045)
    d_phi.GetYaxis().SetLabelSize(0.020)

    d_z0_3d.SetLineColor(ROOT.kBlack)
    d_z0_3d.SetLineWidth(3)
    d_z0_3d.GetListOfFunctions().Add(ROOT.TNamed("Description", "Comparison on z0 of a track between CDC 2D fitter output and MC."))
    d_z0_3d.GetListOfFunctions().Add(ROOT.TNamed("Contact", "ytlai@post.kek.jp"))
    d_z0_3d.GetListOfFunctions().Add(ROOT.TNamed("Check", "A Gaussian peak at 0 with small tail."))
    d_z0_3d.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", "shifter"))

    d_z0_3d.GetXaxis().SetTitle("#Deltaz0 [cm]")
    d_z0_3d.GetYaxis().SetTitle("Events/(1 cm)")
    d_z0_3d.GetYaxis().SetTitleOffset(1.4)
    d_z0_3d.GetXaxis().SetTitleSize(0.045)
    d_z0_3d.GetYaxis().SetLabelSize(0.020)

    d_z0_nn.SetLineColor(ROOT.kBlack)
    d_z0_nn.SetLineWidth(3)
    d_z0_nn.GetListOfFunctions().Add(ROOT.TNamed("Description", "Comparison on z0 of a track between CDC Neuro output and MC."))
    d_z0_nn.GetListOfFunctions().Add(ROOT.TNamed("Contact", "ytlai@post.kek.jp"))
    d_z0_nn.GetListOfFunctions().Add(ROOT.TNamed("Check", "A Gaussian peak at 0 with small tail."))
    d_z0_nn.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", "shifter"))

    d_z0_nn.GetXaxis().SetTitle("#Deltaz0 [cm]")
    d_z0_nn.GetYaxis().SetTitle("Events/(1 cm)")
    d_z0_nn.GetYaxis().SetTitleOffset(1.4)
    d_z0_nn.GetXaxis().SetTitleSize(0.045)
    d_z0_nn.GetYaxis().SetLabelSize(0.020)

    d_E_ECL.SetLineColor(ROOT.kBlack)
    d_E_ECL.SetLineWidth(3)
    d_E_ECL.GetListOfFunctions().Add(ROOT.TNamed("Description", "Comparison on deposit energy between ECL cluster output and MC."))
    d_E_ECL.GetListOfFunctions().Add(ROOT.TNamed("Contact", "ytlai@post.kek.jp"))
    d_E_ECL.GetListOfFunctions().Add(ROOT.TNamed("Check", "A peak around -0.5 ~ 0 with a tail toward -6."))
    d_E_ECL.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", "shifter"))

    d_E_ECL.GetXaxis().SetTitle("#DeltaE [GeV]")
    d_E_ECL.GetYaxis().SetTitle("Events/(0.12 GeV)")
    d_E_ECL.GetYaxis().SetTitleOffset(1.4)
    d_E_ECL.GetXaxis().SetTitleSize(0.045)
    d_E_ECL.GetYaxis().SetLabelSize(0.020)

    def initialize(self):
        m_dbinput = Belle2.PyDBObj('TRGGDLDBInputBits')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')
        n_inbit = m_dbinput.getninbit()
        n_outbit = m_dbftdl.getnoutbit()
        trg_read.hist_inbit.GetXaxis().SetRangeUser(0, n_inbit)
        trg_read.hist_inbit.GetXaxis().SetLabelSize(0.02)
        trg_read.hist_outbit.GetXaxis().SetRangeUser(0, n_outbit)
        trg_read.hist_outbit.GetXaxis().SetLabelSize(0.02)
        for i in range(320):
            inbitname = m_dbinput.getinbitname(i)
            trg_read.hist_inbit.GetXaxis().SetBinLabel(trg_read.hist_inbit.GetXaxis().FindBin(i + 0.5), inbitname)
            outbitname = m_dbftdl.getoutbitname(i)
            trg_read.hist_outbit.GetXaxis().SetBinLabel(trg_read.hist_outbit.GetXaxis().FindBin(i + 0.5), outbitname)

    def beginRun(self):
        trg_read.hist_inbit.Reset()
        trg_read.hist_outbit.Reset()
        trg_read.h_Esum_ECL.Reset()
        trg_read.h_E_ECL.Reset()
        trg_read.h_theta_ECL.Reset()
        trg_read.h_thetaID_ECL.Reset()
        trg_read.h_phi_ECL.Reset()

    def event(self):

        event_meta = Belle2.PyStoreObj('EventMetaData')
        trg_summary = Belle2.PyStoreObj('TRGSummary')
        clusters = Belle2.PyStoreArray('TRGECLClusters')
        klmSummary = Belle2.PyStoreObj('KLMTrgSummary')

        for i in range(320):
            if(trg_summary.testInput(i)):
                trg_read.hist_inbit.Fill(i + 0.5)
            if(trg_summary.testFtdl(i)):
                trg_read.hist_outbit.Fill(i + 0.5)

        etot = 0
        for cluster in clusters:
            x = cluster.getPositionX()
            y = cluster.getPositionY()
            z = cluster.getPositionZ()
            e = cluster.getEnergyDep()
            trg_read.h_E_ECL.Fill(e)
            etot += e
            vec = ROOT.TVector3(x, y, z)
            trg_read.h_theta_ECL.Fill(vec.Theta() * Fac)
            trg_read.h_thetaID_ECL.Fill(cluster.getMaxTCId())
            trg_read.h_phi_ECL.Fill(vec.Phi() * Fac)

        if etot > 0:
            trg_read.h_Esum_ECL.Fill(etot)

        trg_read.h_sector_BKLM.Fill(klmSummary.getBKLM_n_trg_sectors())
        trg_read.h_sector_EKLM.Fill(klmSummary.getEKLM_n_trg_sectors())

    def endRun(self):
        print('end')

    def terminate(self):

        trg_read.hist_inbit.Write()
        trg_read.hist_outbit.Write()
        trg_read.h_Esum_ECL.Write()
        trg_read.h_E_ECL.Write()
        trg_read.h_theta_ECL.Write()
        trg_read.h_thetaID_ECL.Write()
        trg_read.h_phi_ECL.Write()
        trg_read.h_sector_BKLM.Write()
        trg_read.h_sector_EKLM.Write()
        trg_read.d_w.Write()
        trg_read.d_phi.Write()
        trg_read.d_z0_3d.Write()
        trg_read.d_z0_nn.Write()
        trg_read.d_E_ECL.Write()
        trg_read.root_file.Close()


main = create_path()

root_input = basf2.register_module('RootInput')

root_input.param(
    'inputFileName',
    '../TRGValidationGen.root')
root_input.param('branchNames', ['EventMetaData', 'TRGECLClusters', 'TRGSummary', 'KLMTrgSummary'])
main.add_module(root_input)
main.add_module(trg_read())

process(main)
