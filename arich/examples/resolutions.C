#include <stdlib.h>
#include "TH1F.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TVirtualPad.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TTree.h"
#include "TArc.h"
#include "TVector3.h"
#include "TString.h"

/** ROOT script for ARICH performance test
  *
  * Plots the results of the track and momentum resolution
  * obtained from ARICHAnalysis module.
  */ 

void resolutions(TString filename="extArichTest.root")
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);

  TFile* file = new TFile(filename, "READ");
  TTree* tree = (TTree*) file->Get("tree");
  
  Int_t m_eventNo;
  Float_t m_chi2;
  Int_t m_pdg;
  Float_t m_logl[5];
  Float_t m_truePosition[3];
  Float_t m_position[3];
  Float_t m_trueMomentum[3];
  Float_t m_momentum[3];
  
  tree->SetBranchAddress("m_eventNo", &m_eventNo);
  tree->SetBranchAddress("m_chi2", &m_chi2);
  tree->SetBranchAddress("m_pdg", &m_pdg);
  tree->SetBranchAddress("m_logl", &m_logl);
  tree->SetBranchAddress("m_truePosition", &m_truePosition);
  tree->SetBranchAddress("m_position", &m_position);
  tree->SetBranchAddress("m_trueMomentum", &m_trueMomentum);
  tree->SetBranchAddress("m_momentum", &m_momentum);
  
  TH1I* h_stats = new TH1I("h_stats", "Statistics;;No. of tracks", 3, 0, 3);
  h_stats->GetXaxis()->SetBinLabel(1,"");
  h_stats->GetXaxis()->SetBinLabel(2,"Extrapolated");
  h_stats->GetXaxis()->SetBinLabel(3,"Condition pValue>0.05");
  TH1I* h_pdg = new TH1I("h_pdg", "Generated particles passed the condition;PDG code;No. of tracks", 100, -1000, 1000);
  TH2F* h_positionXY = new TH2F("h_positionXY","Reconstructed tracks in X-Y plane;x [cm];y [cm]",100,-120,120,100,-120,120);
  TH1F* h_trueMomentum  = new TH1F("h_trueMomentum", "MC momentum;p_true [GeV]", 100, 0, 5);
  TH2F* h_momentumResolution = new TH2F("h_momentumResolution", "True momentum vs. reconstructed (ext);p_true [GeV];p_reco [GeV]", 100, 0,5, 100, 0,5);
  TH2F* h_resPositionMomentumX = new TH2F("h_resPositionMomentumX","x vs p;p_true [GeV];",100,0,5,100,-2,2);
  TH2F* h_resPositionMomentumY = new TH2F("h_resPositionMomentumY","y vs p;p_true [GeV];",100,0,5,100,-2,2);
  TH2F* h_directionX = new TH2F("h_directionX","Direction X;p_true [GeV];cos #alpha", 100, 0, 5, 100, -1, 1);
  TH2F* h_directionY = new TH2F("h_directionY","Direction Y;p_true [GeV];sin #alpha", 100, 0, 5, 100, -1, 1);
  //TH1F h_true = TH1F("h_true", "Chi2 probability > 5%;;Particles passed cond", 2,0,2);
  TH1F* h_theta = new TH1F("h_theta","Direction #theta;p_true [GeV];",100,0,5);
  TH1F* h_phi = new TH1F("h_theta","Direction #phi;p_true [GeV];",100,0,5);
  TH1F h_loglPi = TH1F("h_loglPi", "#pi likelihood", 100, -5000, 1000);
  TH1F h_loglK = TH1F("h_loglK", "K likelihood", 100, -5000, 1000);
  TH1F h_logDiff("h_logDiff", "Likelihood diff;L_K-L_#pi;Tracks", 100, -1000,1000);
  
  Int_t nEntries = tree->GetEntries();
  for (int i=0; i<nEntries; i++) {
    tree->GetEntry(i);
    h_stats->Fill(1,1);
    const int condition = m_chi2 > 0.05;
    if(condition) {
      h_stats->Fill(2,1);
      h_pdg->Fill(m_pdg);
      
      h_positionXY->Fill(m_position[0], m_position[1]);
      Float_t resX = m_truePosition[0] - m_position[0];
      Float_t resY = m_truePosition[1] - m_position[1];
      
      TVector3 recoMomentum(m_momentum[0], m_momentum[1], m_momentum[2]);    
      TVector3 trueMomentum(m_trueMomentum[0], m_trueMomentum[1], m_trueMomentum[2]);
      
      TVector3 v_dp = trueMomentum.Unit() - recoMomentum.Unit();
      Float_t true_p = trueMomentum.Mag();
      h_trueMomentum->Fill(true_p);
      h_momentumResolution->Fill(true_p, recoMomentum.Mag());
      h_resPositionMomentumX->Fill(true_p, resX);
      h_resPositionMomentumY->Fill(true_p, resY);
      h_directionX->Fill(true_p, v_dp.X());
      h_directionY->Fill(true_p, v_dp.Y());
      
      Float_t theta = recoMomentum.Theta() - trueMomentum.Theta();
      Float_t phi = recoMomentum.Phi() - trueMomentum.Phi();
      h_theta->Fill(true_p, theta);
      h_phi->Fill(true_p, phi);
      h_loglPi.Fill(m_logl[2]);
      h_loglK.Fill(m_logl[3]);
      h_logDiff.Fill(m_logl[3] - m_logl[2]);
      }
    }

  TF1* f_gaus1 = new TF1("f_gaus1", "gaus", -2,2);
  //f_gaus1->SetParameter(1,0);
  h_resPositionMomentumX->FitSlicesY(f_gaus1,11,99);
  TH1F* h_resPositionMomentumX_1 = (TH1F*) gDirectory->Get("h_resPositionMomentumX_1");
  TH1F* h_resPositionMomentumX_2 = (TH1F*) gDirectory->Get("h_resPositionMomentumX_2");
  //f_gaus1->SetParameter(1,0);
  h_resPositionMomentumY->FitSlicesY(f_gaus1,11,99);
  TH1F* h_resPositionMomentumY_1 = (TH1F*) gDirectory->Get("h_resPositionMomentumY_1");
  TH1F* h_resPositionMomentumY_2 = (TH1F*) gDirectory->Get("h_resPositionMomentumY_2");
  TH1D* h_resProjection = h_resPositionMomentumX->ProjectionY("Projection at 0.5 GeV",11,11);
  
  TH1D* h_momentumProjection = h_momentumResolution->ProjectionY("Projection at 4.0 GeV",80,80);
  TF1* f_gaus = new TF1("f_gaus", "gaus", 0, 6);
  h_momentumResolution->Divide(h_trueMomentum);
  h_momentumResolution->FitSlicesY(f_gaus, 11, 99);
  TH1F* h_momentumResolution_1 = (TH1F*) gDirectory->Get("h_momentumResolution_1");
  TH1F* h_momentumResolution_2 = (TH1F*) gDirectory->Get("h_momentumResolution_2");
  
  TF1* f_gaus2 = new TF1("f_gaus2", "gaus", -1,1);
  f_gaus2->SetParameter(0,100);
  f_gaus2->SetParameter(1, 0);
  f_gaus2->SetParameter(2, 0.01);
  h_directionX->FitSlicesY(f_gaus2, 11, 99);
  TH1F* h_directionX_1 = (TH1F*) gDirectory->Get("h_directionX_1");
  TH1F* h_directionX_2 = (TH1F*) gDirectory->Get("h_directionX_2");
  h_directionY->FitSlicesY(f_gaus2, 11, 99);
  TH1F* h_directionY_1 = (TH1F*) gDirectory->Get("h_directionY_1");
  TH1F* h_directionY_2 = (TH1F*) gDirectory->Get("h_directionY_2");
   
  TCanvas canvas1 = TCanvas("canvas1","Events",1000,1000);
  canvas1.Divide(2,2);
  canvas1.cd(1);
  h_stats->GetYaxis()->SetTitleOffset(1.4);
  h_stats->Draw();
  canvas1.cd(2);
  canvas1.cd(3);
  h_pdg->GetYaxis()->SetTitleOffset(1.4);
  h_pdg->Draw();
  canvas1.cd(4);
  h_positionXY->GetYaxis()->SetTitleOffset(1.2);
  h_positionXY->Draw("colz");
  canvas1.Print("resolutions.pdf(");
  

  
  TCanvas canvas4 = TCanvas("canvas4", "Track resolution",1000,1000);
  canvas4.Divide(2,4);
  canvas4.cd(1);
  h_resPositionMomentumX_1->SetTitle("Track hit resolution in X;p [GeV];#mu_x [cm]");
  h_resPositionMomentumX_1->GetYaxis()->SetTitleOffset(1.3);
  h_resPositionMomentumX_1->Draw();
  canvas4.cd(2);
  h_resPositionMomentumX_2->SetTitle("Track hit error ;p [GeV];#sigma_x [cm]");
  h_resPositionMomentumX_2->Draw();
  canvas4.cd(3);
  h_resPositionMomentumY_1->SetTitle("Track hit resolution in Y;p [GeV];#mu_y [cm]");
  h_resPositionMomentumY_1->GetYaxis()->SetTitleOffset(1.3);
  h_resPositionMomentumY_1->Draw();
  canvas4.cd(4);
  h_resPositionMomentumY_2->SetTitle("Track hit error ;p[GeV];#sigma_y [cm]");
  h_resPositionMomentumY_2->Draw();
  canvas4.cd(5);
  h_directionX_1->SetTitle("Direction resolution in X;p[GeV];#mu_sx [rad]");
  h_directionX_1->Draw();
  //h_theta->Draw();
  canvas4.cd(6);
  h_directionX_2->SetTitle("Direction error;p [GeV];#sigma_sx [rad]");
  h_directionX_2->GetYaxis()->SetTitleOffset(1.4);
  h_directionX_2->Draw();
  canvas4.cd(7);
  h_directionY_1->SetTitle("Direction resolution in Y;p [GeV];#mu_sy [rad]");
  h_directionY_1->Draw();
  //h_phi->Draw();
  canvas4.cd(8);
  h_directionY_2->SetTitle("Direction error ;p [GeV];#sigma_sy [rad]");
  h_directionY_2->GetYaxis()->SetTitleOffset(1.4);
  h_directionY_2->Draw();
  canvas4.Print("resolutions.pdf");
  
  TCanvas canvas3 = TCanvas("canvas3","Hit resolution",1000,1000);
  canvas3.Divide(2,2);
  canvas3.cd(1);
  h_theta->Draw();
  canvas3.cd(2);
  h_phi->Draw();
  canvas3.cd(3);
  h_resPositionMomentumX->Draw("colz");
  canvas3.cd(4);
  h_directionX->Draw("colz");
  canvas3.Print("resolutions.pdf");
  
    TCanvas canvas2 = TCanvas("canvas2","Momentum resolution",1000,1000);
  canvas2.Divide(2,2);
  canvas2.cd(3);
  h_momentumResolution_1->SetTitle("True momentum vs. reconstructed fit;p_true [GeV];p_reco [GeV]");
  h_momentumResolution_1->Draw();
  canvas2.cd(4);
  h_momentumResolution_2->SetTitle("Relative error;p_true [GeV];#sigma_p/p");
  h_momentumResolution_2->GetYaxis()->SetTitleOffset(1.5);
  h_momentumResolution_2->Draw();
  canvas2.cd(1);
  h_momentumResolution->Draw();
  canvas2.cd(2);
  h_momentumProjection->SetTitle("Projection of bin at 4.0 GeV;p_reco [GeV];No. of tracks");
  h_momentumProjection->GetYaxis()->SetTitleOffset(1.3);
  h_momentumProjection->Draw();
  canvas2.Print("resolutions.pdf)");
  
  return;
}
