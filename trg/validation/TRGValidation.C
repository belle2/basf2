/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>TRGValidationGen.root</input>
  <output>TRGValidation.root</output>
  <contact>Yun-Tsung Lai, ytlai@post.kek.jp</contact>
  <description>A script to generate histogram plots for trg validation.</description>
</header>
*/

#include "TFile.h"
#include "TCut.h"
#include "TChain.h"
#include "TH1.h"
#include "TH1F.h"
#include "TStyle.h"

void TRGValidation()
{
  TChain* tree = new TChain("tree");
  tree->Add("../TRGValidationGen.root");

  TFile* output = TFile::Open("TRGValidation.root", "recreate");
  gStyle->SetOptStat(kFALSE);

  TCut mc("abs(MCParticles.m_pdg)==11&&MCParticles.m_status==11");
  TH1F* d_w = new TH1F("d_w", "#Deltaw of CDC 2D finder, w = 0.00449/p_{t}", 50, -0.02, 0.02);
  TH1F* d_w_2 = new TH1F("d_w_2", "d_w_2", 50, -0.02, 0.02);
  TH1F* d_phi = new TH1F("d_phi", "#Delta#phi of CDC 2D finder", 50, -0.5, 0.5);
  TH1F* d_phi_2 = new TH1F("d_phi_2", "d_phi_2", 50, -0.5, 0.5);
  TH1F* d_phi_3 = new TH1F("d_phi_3", "d_phi_3", 50, -0.5, 0.5);
  TH1F* d_z0_3d = new TH1F("d_z0_3d", "#Deltaz0 of CDC 3D fitter", 60, -30, 30);
  TH1F* d_z0_nn = new TH1F("d_z0_nn", "#Deltaz0 of CDC Neuro", 60, -30, 30);
  TH1F* d_E_ECL = new TH1F("d_E_ECL", "#DeltaE of ECL clustering", 50, -6, 0);

  tree->Draw("TRGCDC2DFinderTracks.m_omega-0.00449/sqrt(MCParticles.m_momentum_x*MCParticles.m_momentum_x+MCParticles.m_momentum_y*MCParticles.m_momentum_y)>>d_w",
             "MCParticles.m_pdg<0" && mc);
  tree->Draw("TRGCDC2DFinderTracks.m_omega+0.00449/sqrt(MCParticles.m_momentum_x*MCParticles.m_momentum_x+MCParticles.m_momentum_y*MCParticles.m_momentum_y)>>d_w_2",
             "MCParticles.m_pdg>0" && mc);
  tree->Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>>d_phi",
             "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
             "fabs(TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x))<3.1415936" && mc);
  tree->Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)-3.1415936>>d_phi_2",
             "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
             "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)>=3.1415936" && mc);
  tree->Draw("TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)+3.1415936>>d_phi_2",
             "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
             "TRGCDC2DFinderTracks.m_phi0-atan(MCParticles.m_momentum_y/MCParticles.m_momentum_x)<=-3.1415936" && mc);

  tree->Draw("TRGCDC3DFitterTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_3d", mc);
  tree->Draw("TRGCDCNeuroTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_nn", mc);

  tree->Draw("TRGECLClusters.m_edep-MCParticles.m_energy>>d_E_ECL", mc);

  d_w->Add(d_w_2);
  d_w->SetLineColor(kBlack);
  d_w->SetLineWidth(3);
  d_w->GetListOfFunctions()->Add(new TNamed("Description",
                                            "Comparison on w (=0.00449/pt) of a track between CDC 2D finder output and MC."));
  d_w->GetListOfFunctions()->Add(new TNamed("Contact", "ytlai@post.kek.jp"));
  d_w->GetListOfFunctions()->Add(new TNamed("Check", "A clear peak at 0 with tail."));
  d_w->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  d_w->GetXaxis()->SetTitle("#Deltaw");
  d_w->GetYaxis()->SetTitle("Events/(0.08)");
  d_w->GetYaxis()->SetTitleOffset(1.4);
  d_w->GetXaxis()->SetTitleSize(0.045);
  d_w->GetYaxis()->SetLabelSize(0.020);

  d_phi->Add(d_phi_2);
  d_phi->Add(d_phi_3);
  d_phi->SetLineColor(kBlack);
  d_phi->SetLineWidth(3);
  d_phi->GetListOfFunctions()->Add(new TNamed("Description", "Comparison on phi_i of a track between CDC 2D finder output and MC."));
  d_phi->GetListOfFunctions()->Add(new TNamed("Contact", "ytlai@post.kek.jp"));
  d_phi->GetListOfFunctions()->Add(new TNamed("Check", "A Gaussian peak at 0."));
  d_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  d_phi->GetXaxis()->SetTitle("#Delta#phi [rad]");
  d_phi->GetYaxis()->SetTitle("Events/(0.02 rad)");
  d_phi->GetYaxis()->SetTitleOffset(1.4);
  d_phi->GetXaxis()->SetTitleSize(0.045);
  d_phi->GetYaxis()->SetLabelSize(0.020);

  d_z0_3d->SetLineColor(kBlack);
  d_z0_3d->SetLineWidth(3);
  d_z0_3d->GetListOfFunctions()->Add(new TNamed("Description", "Comparison on z0 of a track between CDC 2D fitter output and MC."));
  d_z0_3d->GetListOfFunctions()->Add(new TNamed("Contact", "ytlai@post.kek.jp"));
  d_z0_3d->GetListOfFunctions()->Add(new TNamed("Check", "A Gaussian peak at 0 with small tail."));
  d_z0_3d->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  d_z0_3d->GetXaxis()->SetTitle("#Deltaz0 [cm]");
  d_z0_3d->GetYaxis()->SetTitle("Events/(1 cm)");
  d_z0_3d->GetYaxis()->SetTitleOffset(1.4);
  d_z0_3d->GetXaxis()->SetTitleSize(0.045);
  d_z0_3d->GetYaxis()->SetLabelSize(0.020);

  d_z0_nn->SetLineColor(kBlack);
  d_z0_nn->SetLineWidth(3);
  d_z0_nn->GetListOfFunctions()->Add(new TNamed("Description", "Comparison on z0 of a track between CDC Neuro output and MC."));
  d_z0_nn->GetListOfFunctions()->Add(new TNamed("Contact", "ytlai@post.kek.jp"));
  d_z0_nn->GetListOfFunctions()->Add(new TNamed("Check", "A Gaussian peak at 0 with small tail."));
  d_z0_nn->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  d_z0_nn->GetXaxis()->SetTitle("#Deltaz0 [cm]");
  d_z0_nn->GetYaxis()->SetTitle("Events/(1 cm)");
  d_z0_nn->GetYaxis()->SetTitleOffset(1.4);
  d_z0_nn->GetXaxis()->SetTitleSize(0.045);
  d_z0_nn->GetYaxis()->SetLabelSize(0.020);

  d_E_ECL->SetLineColor(kBlack);
  d_E_ECL->SetLineWidth(3);
  d_E_ECL->GetListOfFunctions()->Add(new TNamed("Description", "Comparison on deposit energy between ECL cluster output and MC."));
  d_E_ECL->GetListOfFunctions()->Add(new TNamed("Contact", "ytlai@post.kek.jp"));
  d_E_ECL->GetListOfFunctions()->Add(new TNamed("Check", "A peak around -0.5 ~ 0 with a tail toward -6."));
  d_E_ECL->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  d_E_ECL->GetXaxis()->SetTitle("#DeltaE [GeV]");
  d_E_ECL->GetYaxis()->SetTitle("Events/(0.12 GeV)");
  d_E_ECL->GetYaxis()->SetTitleOffset(1.4);
  d_E_ECL->GetXaxis()->SetTitleSize(0.045);
  d_E_ECL->GetYaxis()->SetLabelSize(0.020);

  d_w->Write();
  d_phi->Write();
  d_z0_3d->Write();
  d_z0_nn->Write();
  d_E_ECL->Write();
  output->Close();
  delete output;


}
