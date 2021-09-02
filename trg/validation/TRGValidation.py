#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################
# test for module
################################################

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


class trg_read(basf2.Module):

    root_file = ROOT.TFile('TRGValidation.root', 'recreate')

    ROOT.gStyle.SetOptStat(ROOT.kFALSE)

    hist_inbit = ROOT.TH1F('hin', 'trigger input bits', 320, 0, 320)
    hist_outbit = ROOT.TH1F('hout', 'trigger output bits', 320, 0, 320)

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

    tree.Draw("(TRGCDC2DFinderTracks.m_omega - 0.00449) /
              (sqrt(MCParticles.m_momentum_x * MCParticles.m_momentum_x
                    + MCParticles.m_momentum_y * MCParticles.m_momentum_y)) >> d_w",
              "MCParticles.m_pdg<0&&" +
              mc)
    tree.Draw("(TRGCDC2DFinderTracks.m_omega + 0.00449) /
              sqrt(MCParticles.m_momentum_x * MCParticles.m_momentum_x
                   + MCParticles.m_momentum_y * MCParticles.m_momentum_y) >> d_w_2",
              "MCParticles.m_pdg>0 && " +
              mc)
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
        print('initialize')
        m_dbinput = Belle2.PyDBObj('TRGGDLDBInputBits')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')
        n_inbit = m_dbinput.getninbit()
        n_outbit = m_dbftdl.getnoutbit()
        print(n_inbit)
        print(n_outbit)
        trg_read.hist_inbit.GetXaxis().SetRangeUser(0, n_inbit)
        trg_read.hist_outbit.GetXaxis().SetRangeUser(0, n_outbit)
        for i in range(320):
            inbitname = m_dbinput.getinbitname(i)
            trg_read.hist_inbit.GetXaxis().SetBinLabel(trg_read.hist_inbit.GetXaxis().FindBin(i + 0.5), inbitname)
            outbitname = m_dbftdl.getoutbitname(i)
            trg_read.hist_outbit.GetXaxis().SetBinLabel(trg_read.hist_outbit.GetXaxis().FindBin(i + 0.5), outbitname)

    def beginRun(self):
        print('beginRun')

    def event(self):

        event_meta = Belle2.PyStoreObj('EventMetaData')
        trg_summary = Belle2.PyStoreObj('TRGSummary')
        m_dbinput = Belle2.PyDBObj('TRGGDLDBInputBits')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')

        runno = event_meta.getRun()
        evtno = event_meta.getEvent()

        for i in range(320):
            if(trg_summary.testInput(i)):
                trg_read.hist_inbit.Fill(i + 0.5)
            if(trg_summary.testFtdl(i)):
                trg_read.hist_outbit.Fill(i + 0.5)

    def endRun(self):
        print('endRun')

    def terminate(self):
        print('terminate')

        trg_read.hist_inbit.Write()
        trg_read.hist_outbit.Write()
        trg_read.d_w.Write()
        trg_read.d_phi.Write()
        trg_read.d_z0_3d.Write()
        trg_read.d_z0_nn.Write()
        trg_read.d_E_ECL.Write()
        trg_read.root_file.Close()


main = create_path()

root_input = basf2.register_module('RootInput')

root_input.param('inputFileName', '../TRGValidationGen.root')
root_input.param('branchNames', ['EventMetaData', 'TRGSummary'])
main.add_module(root_input)
main.add_module(trg_read())

process(main)
