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
#include "TMath.h"
#include "TGaxis.h"
#include "TLegend.h"

/** ROOT script for ARICH performance test
  *
  * Plots the results of the track and momentum resolution
  * obtained from ARICHAnalysis module.
  */ 

void resolutions(TString filename="extArichTest.root")
{
  // set draw style
	gStyle->SetOptStat("e");
	gStyle->SetPalette(1, 0);
	
	gStyle->SetPaperSize(TStyle::kA4);
	gStyle->SetStatBorderSize(1);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameFillColor(0);
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0);
	gStyle->SetCanvasColor(0);
	gStyle->SetStatColor(0);
	gStyle->SetOptFit(1111);
	gStyle->SetPadRightMargin(0.1);
	TGaxis::SetMaxDigits(3);

  TFile* file = new TFile(filename, "READ");
  TTree* tree = (TTree*) file->Get("tree");
  
  //Variables
  Int_t m_eventNo;
  Float_t m_chi2;
  Int_t m_pdg;
  Int_t m_charge;
  Int_t m_flag;
  Float_t m_lifetime;
  Float_t m_decayVertex[3];
  Float_t m_logl[5];
  Float_t m_truePosition[3];
  Float_t m_position[3];
  Float_t m_trueMomentum[3];
  Float_t m_momentum[3];
  
  tree->SetBranchAddress("m_eventNo", &m_eventNo);
  tree->SetBranchAddress("m_chi2", &m_chi2);
  tree->SetBranchAddress("m_pdg", &m_pdg);
  tree->SetBranchAddress("m_charge", &m_charge);
  tree->SetBranchAddress("m_flag", &m_flag);
  tree->SetBranchAddress("m_lifetime", &m_lifetime);
  tree->SetBranchAddress("m_decayVertex", &m_decayVertex);
  tree->SetBranchAddress("m_logl", &m_logl);
  tree->SetBranchAddress("m_truePosition", &m_truePosition);
  tree->SetBranchAddress("m_position", &m_position);
  tree->SetBranchAddress("m_trueMomentum", &m_trueMomentum);
  tree->SetBranchAddress("m_momentum", &m_momentum);
  
  // Histograms
  TH1I* h_stats = new TH1I("h_stats", "Statistics;;No. of tracks", 2, 1, 3);
  h_stats->GetXaxis()->SetBinLabel(1,"Extrapolated");
  h_stats->GetXaxis()->SetBinLabel(2,"Condition pValue>0.01");
  //h_stats->GetXaxis()->SetBinLabel(3,"Primary");
  TH1I* h_pdg = new TH1I("h_pdg", "Generated particles passed the condition;PDG code;No. of tracks", 100, -500, 500);
  TH1I* h_generatedK = new TH1I("h_generatedK","PDG code;No. of tracks", 100, -500,500);
  TH1I* h_generatedPi = new TH1I("h_generatedPi","PDG code;No. of tracks", 100, -500,500);
  
  TH2F* h_positionXY = new TH2F("h_positionXY","Reconstructed tracks in X-Y plane;x [cm];y [cm]",100,-120,120,100,-120,120);
  TH1F* h_trueMomentum  = new TH1F("h_trueMomentum", "MC momentum;p_true [GeV]", 100, 0, 5);
  TH2F* h_momentumResolution = new TH2F("h_momentumResolution", "True momentum vs. reconstructed (ext);p_true [GeV];p_true - p_reco [GeV]", 100, 0,5, 200, -0.2,0.2);
  TH2F* h_resPositionMomentumX = new TH2F("h_resPositionMomentumX","Resolution in x;p_true [GeV];x_true - x_reco [cm]",100,0,5,200,-0.4,0.4);
  TH2F* h_resPositionMomentumY = new TH2F("h_resPositionMomentumY","Resolution in y;p_true [GeV];y_true - y_reco [cm]",100,0,5,200,-0.4,0.4);
  
  TH2F* h_theta = new TH2F("h_theta","Polar angle #theta resolution;p_true [GeV];#theta - #theta_reco [rad]",100,0,5, 200, -0.04,0.04);
  TH2F* h_phi = new TH2F("h_phi","Azimuthal angle #phi resolution;p_true [GeV];#phi - #phi_reco [rad]",100,0,5, 200, -0.04, 0.04);
  TH2F* h_cosTheta = new TH2F("h_cosTheta","Cos #theta;p_true [GeV];",100,0,5, 200, -0.02,0.02);
  TH2F* h_pt = new TH2F("h_pt","pt;p_true [GeV];pt - pt_reco [GeV]",100,0,5, 200, -0.05,0.05);
  
  TH1F h_loglPi = TH1F("h_loglPi", "#pi likelihood", 100, -5000, 1000);
  TH1F h_loglK = TH1F("h_loglK", "K likelihood", 100, -5000, 1000);
  TH1F h_logDiff("h_logDiff", "Likelihood diff;L_K-L_#pi;Tracks", 100, -1000,1000);
  
  //Process entries
  Int_t nEntries = tree->GetEntries();
  for (int i=0; i<nEntries; i++) {
    tree->GetEntry(i);
    h_stats->Fill(1,1);
    const int condition1 = m_chi2 > 0.01; // pValue of the fit > 1%
    const int condition2 = m_flag & 1; // primary in generator
    if (condition1) h_stats->Fill(2,1);
    if(condition1 and condition2) {
      //h_stats->Fill(3,1);
      h_pdg->Fill(m_pdg);
      if (abs(m_pdg) == 211) h_generatedPi->Fill(m_pdg);
      if (abs(m_pdg) == 321) h_generatedK->Fill(m_pdg);
      
      h_positionXY->Fill(m_position[0], m_position[1]);
      Float_t resX = m_truePosition[0] - m_position[0];
      Float_t resY = m_truePosition[1] - m_position[1];
      
      TVector3 recoMomentum(m_momentum[0], m_momentum[1], m_momentum[2]);    
      TVector3 trueMomentum(m_trueMomentum[0], m_trueMomentum[1], m_trueMomentum[2]);
      //TVector3 dp = trueMomentum.Unit() - recoMomentum.Unit();
      Float_t true_p = trueMomentum.Mag();
      h_trueMomentum->Fill(true_p);
      h_momentumResolution->Fill(true_p, true_p - recoMomentum.Mag());
      h_cosTheta->Fill(true_p, trueMomentum.CosTheta() - recoMomentum.CosTheta());
      h_pt->Fill(true_p, trueMomentum.Pt() - recoMomentum.Pt());
      h_theta->Fill(true_p, recoMomentum.Theta() - trueMomentum.Theta());
      h_phi->Fill(true_p, recoMomentum.Phi() - trueMomentum.Phi());
      
      h_resPositionMomentumX->Fill(true_p, resX);
      h_resPositionMomentumY->Fill(true_p, resY);
      
      h_loglPi.Fill(m_logl[2]);
      h_loglK.Fill(m_logl[3]);
      h_logDiff.Fill(m_logl[3] - m_logl[2]);
      
    }
  }
  
  //Draw results
  //
  //Statistics
  TCanvas* canvas1 = new TCanvas("canvas1","Events",3*500,500);
  canvas1->Divide(3,1);
  canvas1->cd(1);
  h_stats->GetYaxis()->SetTitleOffset(1.4);
  //h_stats->GetYaxis()->SetMaxDigits(3);
  h_stats->Draw();
  canvas1->cd(2);
  TLegend* legend1 = new TLegend(0.45,0.7,0.55,0.8);
  legend1->SetFillColor(0);
  legend1->SetBorderSize(0);
  legend1->SetTextSize(0.045);
  legend1->AddEntry(h_generatedPi,"#pi");
  legend1->AddEntry(h_generatedK, "K");
  h_pdg->GetYaxis()->SetTitleOffset(1.4);
  h_pdg->Draw();
  h_generatedPi->SetLineColor(kRed);
  h_generatedPi->Draw("same");
  h_generatedK->SetLineColor(kBlue);
  h_generatedK->Draw("same");
  legend1->Draw("same");
  canvas1->cd(3);
  h_positionXY->GetYaxis()->SetTitleOffset(1.4);
  h_positionXY->Draw("colz");
  canvas1->Print("resolutions1.pdf");
  
  // Momentum
  TH1D* h_momentumProjection = h_momentumResolution->ProjectionY("Projection at 3.0 GeV",60,60);
  TF1* f_gaus = new TF1("f_gaus", "gaus",-0.2,0.2);
  h_momentumResolution->Divide(h_trueMomentum);
  f_gaus->SetParameter(0, h_momentumResolution->GetMaximum());
  f_gaus->SetParameter(1, h_momentumResolution->GetBinCenter(h_momentumResolution->GetMaximumBin()) );
  f_gaus->SetParameter(2, 0.01);
  h_momentumResolution->FitSlicesY(f_gaus, 11, 99);
  TH1F* h_momentumResolution_1 = (TH1F*) gDirectory->Get("h_momentumResolution_1");
  TH1F* h_momentumResolution_2 = (TH1F*) gDirectory->Get("h_momentumResolution_2");
  
  TCanvas* canvas2 = new TCanvas("canvas2","Momentum resolution",2*500,2*500);
  canvas2->Divide(2,2);
  canvas2->cd(1);
  h_momentumResolution->GetYaxis()->SetTitleOffset(1.3);
  h_momentumResolution->Draw("colz");
  canvas2->cd(2);
  h_momentumProjection->SetTitle("Projection of bin at 4.0 GeV;p_reco [GeV];No. of tracks");
  h_momentumProjection->GetYaxis()->SetTitleOffset(1.3);
  h_momentumProjection->Draw();
  canvas2->cd(3);
  h_momentumResolution_1->SetTitle("Momentum resolution mean;p_true [GeV]; #mu [GeV]");
  h_momentumResolution_1->GetYaxis()->SetTitleOffset(1.5);
  h_momentumResolution_1->GetYaxis()->SetRangeUser(-0.04,0.04);
  h_momentumResolution_1->Draw();
  canvas2->cd(4);
  h_momentumResolution_2->SetTitle("Relative error;p_true [GeV];#sigma_p/p");
  h_momentumResolution_2->GetYaxis()->SetTitleOffset(1.5);
  h_momentumResolution_2->GetYaxis()->SetRangeUser(-0.04,0.04);
  h_momentumResolution_2->Draw();
  canvas2->Print("resolutions5.pdf");

  // Position resolution
  TF1* f_gaus1 = new TF1("f_gaus1", "gaus", -0.6,0.6);
  f_gaus1->SetParameter(0, h_resPositionMomentumX->GetMaximum());
  f_gaus1->SetParameter(1, h_resPositionMomentumX->GetBinCenter(h_resPositionMomentumX->GetMaximumBin()) );
  f_gaus1->SetParameter(2, 0.01);
  h_resPositionMomentumX->FitSlicesY(f_gaus1,11,99);
  TH1F* h_resPositionMomentumX_1 = (TH1F*) gDirectory->Get("h_resPositionMomentumX_1");
  TH1F* h_resPositionMomentumX_2 = (TH1F*) gDirectory->Get("h_resPositionMomentumX_2");
  
  f_gaus1->SetParameter(0, h_resPositionMomentumY->GetMaximum());
  f_gaus1->SetParameter(1, h_resPositionMomentumY->GetBinCenter(h_resPositionMomentumY->GetMaximumBin()) );
  f_gaus1->SetParameter(2, 0.01);
  h_resPositionMomentumY->FitSlicesY(f_gaus1,11,99);
  TH1F* h_resPositionMomentumY_1 = (TH1F*) gDirectory->Get("h_resPositionMomentumY_1");
  TH1F* h_resPositionMomentumY_2 = (TH1F*) gDirectory->Get("h_resPositionMomentumY_2");
  
  TH1D* h_resProjection = h_resPositionMomentumX->ProjectionY("Projection at 3.0 GeV",60,60);
    
  TCanvas* canvas4 = new TCanvas("canvas4", "Position resolution",3*500,2*500);
  canvas4->Divide(3,2);
  canvas4->cd(1);
  h_resPositionMomentumX->GetYaxis()->SetTitleOffset(1.4);
  h_resPositionMomentumX->Draw("colz");
  canvas4->cd(4);
  h_resProjection->SetTitle("Resolution in x at 3.0 GeV; p_reco [GeV]; Events");
  h_resProjection->Draw();
  canvas4->cd(2);
  h_resPositionMomentumX_1->SetTitle("Track hit resolution in X;p [GeV];#mu_x [cm]");
  h_resPositionMomentumX_1->GetYaxis()->SetTitleOffset(1.5);
  Double_t sigma = h_resPositionMomentumX_2->GetMaximum();
  sigma *= 1.1;
  h_resPositionMomentumX_1->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_resPositionMomentumX_1->Draw();
  canvas4->cd(3);
  h_resPositionMomentumX_2->SetTitle("Track hit error ;p [GeV];#sigma_x [cm]");
  h_resPositionMomentumX_2->GetYaxis()->SetTitleOffset(1.5);
  h_resPositionMomentumX_2->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_resPositionMomentumX_2->Draw();
  canvas4->cd(5);
  h_resPositionMomentumY_1->SetTitle("Track hit resolution in Y;p [GeV];#mu_y [cm]");
  h_resPositionMomentumY_1->GetYaxis()->SetTitleOffset(1.5);
  sigma = h_resPositionMomentumY_2->GetMaximum();
  sigma *= 1.1;
  h_resPositionMomentumY_1->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_resPositionMomentumY_1->Draw();
  canvas4->cd(6);
  h_resPositionMomentumY_2->SetTitle("Track hit error ;p[GeV];#sigma_y [cm]");
  h_resPositionMomentumY_2->GetYaxis()->SetTitleOffset(1.5);
  h_resPositionMomentumY_2->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_resPositionMomentumY_2->Draw();
  canvas4->Print("resolutions2.pdf");
  
  // Direction resolution
  TF1* f_gaus2 = new TF1("f_gaus2", "gaus", -0.5, 0.5);
  f_gaus2->SetParameter(0, h_theta->GetMaximum() );
  f_gaus2->SetParameter(1, h_theta->GetBinCenter(h_theta->GetMaximum()) );
  f_gaus2->SetParameter(2, 0.001);
  h_theta->FitSlicesY(f_gaus2, 11, 99);
  TH1F* h_theta_1 = (TH1F*) gDirectory->Get("h_theta_1");
  TH1F* h_theta_2 = (TH1F*) gDirectory->Get("h_theta_2");
  
  h_phi->FitSlicesY(f_gaus2, 11, 99);
  f_gaus2->SetParameter(0, h_phi->GetMaximum() );
  f_gaus2->SetParameter(1, h_phi->GetBinCenter(h_phi->GetMaximum()) );
  f_gaus2->SetParameter(2, 0.001);
  TH1F* h_phi_1 = (TH1F*) gDirectory->Get("h_phi_1");
  TH1F* h_phi_2 = (TH1F*) gDirectory->Get("h_phi_2");
  
  TCanvas* canvas3 = new TCanvas("canvas3","Direction resolution",3*500,2*500);
  canvas3->Divide(3,2);
  canvas3->cd(1);
  h_theta->Draw("colz");
  canvas3->cd(2);
  sigma = h_theta_2->GetMaximum();
  sigma *= 1.1;
  h_theta_1->SetTitle("Resolution in #theta;p [GeV];#mu [rad]");
  h_theta_1->GetYaxis()->SetTitleOffset(1.5);
  h_theta_1->GetYaxis()->SetRangeUser(-sigma,sigma);
  h_theta_1->Draw();
  canvas3->cd(3);
  h_theta_2->SetTitle("Resolution in #theta;p [GeV];#sigma [rad]");
  h_theta_2->GetYaxis()->SetTitleOffset(1.5);
  h_theta_2->GetYaxis()->SetRangeUser(-sigma,sigma);
  h_theta_2->Draw();
  canvas3->cd(4);
  h_phi->Draw("colz");
  canvas3->cd(5);
  sigma = h_phi_2->GetMaximum();
  sigma *= 1.1;
  h_phi_1->GetYaxis()->SetRangeUser(-sigma,sigma);
  h_phi_1->GetYaxis()->SetTitleOffset(1.5);
  h_phi_1->SetTitle("Resolution in #phi;p [GeV];#mu [rad]");
  h_phi_1->Draw();
  canvas3->cd(6);
  h_phi_2->SetTitle("Resolution in #phi;p [GeV];#sigma [rad]");
  h_phi_2->GetYaxis()->SetTitleOffset(1.5);
  h_phi_2->GetYaxis()->SetRangeUser(-sigma,sigma);
  h_phi_2->Draw();
  canvas3->Print("resolutions3.pdf");
  
  //Direction2
  TF1* f_gaus3 = new TF1("f_gaus3", "gaus", -0.2,0.2);
  f_gaus3->SetParameter(0, h_cosTheta->GetMaximum());
  f_gaus3->SetParameter(1, h_cosTheta->GetBinCenter(h_cosTheta->GetMaximum()));
  f_gaus3->SetParameter(2, 0.01);
  h_cosTheta->FitSlicesY(f_gaus3, 11, 99);
  TH1F* h_cosTheta_1 = (TH1F*) gDirectory->Get("h_cosTheta_1");
  TH1F* h_cosTheta_2 = (TH1F*) gDirectory->Get("h_cosTheta_2");
  
  f_gaus3->SetParameter(0, h_pt->GetMaximum());
  f_gaus3->SetParameter(1, h_pt->GetBinCenter(h_pt->GetMaximum()));
  f_gaus3->SetParameter(2, 0.001);
  h_pt->FitSlicesY(f_gaus3, 11, 99);
  TH1F* h_pt_1 = (TH1F*) gDirectory->Get("h_pt_1");
  TH1F* h_pt_2 = (TH1F*) gDirectory->Get("h_pt_2");
  
  TCanvas* canvas31 = new TCanvas("canvas31","canvas31",3*500,2*500);
  canvas31->Divide(3,2);
  canvas31->cd(1);
  h_cosTheta->Draw("colz");
  canvas31->cd(2);
  sigma = h_cosTheta_2->GetMaximum();
  sigma *= 1.1;
  h_cosTheta_1->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_cosTheta_1->Draw();
  canvas31->cd(3);
  h_cosTheta_2->Draw();
  canvas31->cd(4);
  sigma = h_pt_2->GetMaximum();
  sigma *= 1.1;
  h_pt_1->GetYaxis()->SetRangeUser(-sigma, sigma);
  h_pt->Draw("colz");
  canvas31->cd(5);
  h_pt_1->Draw();
  canvas31->cd(6);
  h_pt_2->Draw();
  canvas31->Print("resolutions4.pdf");
  
  
  
  return;
}
