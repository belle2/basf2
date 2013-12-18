/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic                                           *
 *                                                                        *
 * Root script for ARICH performance test                                 *
 * Plots the results of the kaon detection efficiency                     *
 * obtained from myArichModule.                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <stdlib.h>
#include "TH1F.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TProfile2D.h"
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

// Global constants
  const Float_t m_lkpcut = 0.01;
  const Int_t m_lbins = 5000;
  const Float_t m_lmax = 2000;
  const Float_t m_lmin = - m_lmax;
  const Int_t m_pbins = 50;
  const Float_t m_pmin = 0;
  const Float_t m_pmax = 5;
  const Float_t m_dp = (m_pmax - m_pmin)/Float_t(m_pbins);
  const Float_t m_rmin = 40;
  const Float_t m_rmax = 113;
  const Int_t m_rbins = 1; 
  const Float_t m_dr = (m_rmax - m_rmin)/Float_t(m_rbins);

/** Function which sets the likelihood cuts
  * 
  * The function takes the 3d histogram K-logL vs pi-logL vs. momentum
  * and sets cuts per every bin of momentum.
  * The number of fake pions is set by global constant m_lkpicut
  */  
void setCuts(const TH3F&, Float_t [], TH1F*);

/** Root script for ARICH performance test
  *
  * Takes the tracks from ARICHAnalysis module and plots the kaon detection 
  * efficiency with the constant pion fake rate
  * inside the ARICH geometric acceptance area.
  *
  * rMin Inner radius of ARICH accpetance
  * rMax Outer radius of ARICH acceptance
  * filename Output file name for the results
  */
void extArichEfficiency(Float_t rMin=51, Float_t rMax=107, 
                        TString filename="extArichTest.root")
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);

  TFile* file = new TFile(filename, "READ");
  TTree* tree = (TTree*) file->Get("tree");
  
  Int_t m_eventNo;
  Int_t m_trackNo;
  Int_t m_pdg;
  Int_t m_charge;
  Int_t m_primary;
  Float_t m_logl[5];
  Int_t m_detPhotons;
  Float_t m_expPhotons[5];
  //Float_t m_truePosition[3];
  Float_t m_position[3];
  //Float_t m_trueMomentum[3];
  Float_t m_momentum[3];

  tree->SetBranchAddress("m_eventNo", &m_eventNo);
  tree->SetBranchAddress("m_trackNo", &m_trackNo);
  tree->SetBranchAddress("m_pdg", &m_pdg);
  tree->SetBranchAddress("m_charge", &m_charge);
  tree->SetBranchAddress("m_flag", &m_primary);
  tree->SetBranchAddress("m_logl", &m_logl);
  tree->SetBranchAddress("m_detPhotons", &m_detPhotons);
  tree->SetBranchAddress("m_expPhotons", &m_expPhotons);
  //tree->SetBranchAddress("m_truePosition", &m_truePosition);
  tree->SetBranchAddress("m_position", &m_position);
  //tree->SetBranchAddress("m_trueMomentum", &m_trueMomentum);
  tree->SetBranchAddress("m_momentum", &m_momentum);
  
  TH1I* h_stats = new TH1I("h_stats", "Statistics;;No. of hits", 2, 1, 3);
  h_stats->GetXaxis()->SetBinLabel(1,"All");
  h_stats->GetXaxis()->SetBinLabel(2,"Primaries");
  TH1I* h_pdg = new TH1I("h_pdg", "Hits identity;PDG code;No. of tracks", 100, -350, 350);
  TH2F* h_positionXY = new TH2F("h_positionXY", "AeroHits in X-Y plane;x [cm];y [cm]",100,-120,120,100,-120,120);
  TH1F* h_momentum  = new TH1F("h_momentum", "Momentum distribution;p_reco [GeV];No. of tracks", 100, 0, 5);
  TH3F h_kaon("h_kaon","h_kaon", m_pbins, m_pmin, m_pmax, m_rbins, m_rmin, m_rmax, m_lbins, m_lmin, m_lmax);
  TH3F h_pion("h_pion","h_pion", m_pbins, m_pmin, m_pmax, m_rbins, m_rmin, m_rmax, m_lbins, m_lmin, m_lmax);
  
  Int_t nEntries = tree->GetEntries();
  Float_t all = 0;
  Float_t primaries = 0;
  for (int i=0; i<nEntries; i++) {
    tree->GetEntry(i);
    all++;
    h_stats->Fill(1,1);
    h_pdg->Fill(m_pdg);
    if(m_primary) {
      primaries++;
      h_stats->Fill(2,1);
      h_positionXY->Fill(m_position[0], m_position[1]);
      TVector3 position( m_position[0], m_position[1], m_position[2]);
      Float_t r = position.XYvector().Mod();
      TVector3 momentum(m_momentum[0], m_momentum[1], m_momentum[2]);
      Float_t p = momentum.Mag();
      h_momentum->Fill(p);

      if (abs(m_pdg) == 321) {
        h_kaon.Fill(p, r, m_logl[3] - m_logl[2]);
      }
      if (abs(m_pdg) == 211) {
        h_pion.Fill(p, r, m_logl[3] - m_logl[2]);
      }
    }
  }
  cout << "All particles: " << all << ", primary: " << primaries << endl;

  // Likelihood cut
  Float_t cut[m_pbins];
  TH1F* h_cuts = new TH1F("h_cuts", "cuts", m_pbins, m_pmin, m_pmax);  
  setCuts(h_pion, cut, h_cuts);
  
  // Final histograms, detection efficiency
  TH2F* h_pionMomentum = new TH2F("h_pionMomentum","Likelihood vs. momentum for #pi;p [GeV];L_K - L_#pi",m_pbins, 0, 5, 100, -1000, 1000);
  TH2F* h_kaonMomentum = new TH2F("h_kaonMomentum","Likelihood vs. momentum for K;p [GeV];L_K - L_#pi", m_pbins, 0, 5, 100, -1000, 1000);
  TH1F* h_detectedPi = new TH1F("h_detectedPi", "Detected #pi identified as K;p [GeV];Fake #pi ratio", m_pbins, 0, 5);
  TH1F* h_allPi = new TH1F("h_allPi", "All pi", m_pbins, 0, 5);
  TH1F* h_detectedK = new TH1F("h_detectedK", "Detected K identified as K;p [GeV];Kaon detection eff. ratio", m_pbins, 0, 5);
  TH1F* h_allK = new TH1F("h_allK", "All K", m_pbins, 0, 5);
  TProfile* p_detPhotonsK = new TProfile("p_detPhotonsK", "Detected photons;p [GeV];Photons", m_pbins, 0, 5);
  TProfile* p_detPhotonsPi = new TProfile("p_detPhotonsPi", "Detected photons;p [GeV];Photons", m_pbins, 0, 5);
  TProfile2D* p_detPhotons = new TProfile2D("p_detPhotons","Detected photons per ring;x [cm];y [cm]", 240, -120.0, 120.0,240,-120,120.0);
  TProfile2D* p_lostPhotonsK = new TProfile2D("p_lostPhotonsK", "Photons produced by K below cut;x [cm];y [cm]", 120, 0.0, 120.0, 120, 0.0, 120.0);
  TProfile2D* p_fakePions = new TProfile2D("p_fakePions", "Photons produced by fake #pi;x [cm];y [cm]", 120, 0.0, 120.0, 120, 0.0, 120.0);
  
  Int_t allPions = 0;
  Int_t allKaons = 0;
  Int_t fakePions = 0;
  Int_t detKaons = 0;
  Int_t lostKaons = 0;
  
  for(Int_t iHit=0; iHit<nEntries; iHit++) {
    tree->GetEntry(iHit);
    if(m_primary) {
      TVector3 position(m_position[0], m_position[1], m_position[2]);
      Float_t r = position.XYvector().Mod();
      Float_t phi = position.XYvector().Phi();
      Float_t sextant = phi - int(phi/(M_PI/3.0)) * (M_PI / 3.0);
      TVector2 rho;
      rho.SetMagPhi(r, sextant);
      TVector3 momentum(m_momentum[0], m_momentum[1], m_momentum[2]);
      Float_t p = momentum.Mag();
      Int_t bin = Int_t((p - m_pmin) / m_dp);
      if(bin<0 or bin>m_pbins) continue;
      if(p<0.01) continue;
      // geometric acceptance of arich
      if(r>rMin and r<rMax) {
        p_detPhotons->Fill(m_position[0], m_position[1], m_detPhotons);
        if(fabs(m_pdg) == 211) {
          allPions++;
          h_allPi->Fill(p,1);
          h_pionMomentum->Fill(p, m_logl[3]-m_logl[2]);
          p_detPhotonsPi->Fill(p, m_detPhotons);
          if(m_logl[3]-m_logl[2] > cut[bin]) {
            fakePions++;
            h_detectedPi->Fill(p,1);
            p_fakePions->Fill(rho.X(), rho.Y(), m_detPhotons);
          }
        } 
        if(fabs(m_pdg) == 321) {
          allKaons++;
          h_allK->Fill(p, 1);
          h_kaonMomentum->Fill(p, m_logl[3]-m_logl[2]);
          p_detPhotonsK->Fill(p, m_detPhotons);
          if(m_logl[3]-m_logl[2] > cut[bin]) {
            detKaons++;
            h_detectedK->Fill(p,1);
          } else if(m_logl[3]-m_logl[2] <= cut[bin]) {
            lostKaons++;
            p_lostPhotonsK->Fill(rho.X(), rho.Y(), m_detPhotons);
          }
        }
      }  
    }
  }
  
  h_detectedK->Sumw2();
  h_allK->Sumw2();
  h_detectedK->Divide(h_allK);
  h_detectedPi->Divide(h_allPi);
  
  TH1D* h_kaonProjection = h_kaonMomentum->ProjectionY("LogL projection at 3.0 GeV",30,31);
  TH1D* h_pionProjection = h_pionMomentum->ProjectionY("LogL projetcion at 3.0 GeV",30,31);
  
  Float_t efficiency = detKaons/Float_t(allKaons);
  Float_t fake = fakePions/Float_t(allPions);
  
  cout << " All pions " << allPions << " fake pions " << fakePions << endl;
  cout << " All kaons " << allKaons << " det kaons  " << detKaons << " lost kaons " << lostKaons << endl;
  cout << " Fake rate " << fake << endl;
  cout << " Kaon det. eff " << efficiency << endl;
   
  TCanvas canvas1 = TCanvas("canvas1","Events",1000,1000);
  canvas1.Divide(2,2);
  canvas1.cd(1);
  h_stats->GetYaxis()->SetTitleOffset(1.4);
  h_stats->Draw();
  canvas1.cd(2);
  h_pdg->GetYaxis()->SetTitleOffset(1.6);
  h_pdg->Draw();
  canvas1.cd(3);
  h_momentum->GetYaxis()->SetTitleOffset(1.4);
  h_momentum->Draw();
  canvas1.cd(4);
  h_positionXY->GetYaxis()->SetTitleOffset(1.2);
  h_positionXY->Draw("colz");
  canvas1.Print("detectionEfficiency1.pdf");
  
  TCanvas canvas2("canvas2", "Detection efficiency 1",1000,1000);
  canvas2.Divide(2,2);
  canvas2.cd(3);
  h_detectedK->GetYaxis()->SetTitleOffset(1.2);
  h_detectedK->Draw("e");
  canvas2.cd(4);
  h_detectedPi->GetYaxis()->SetTitleOffset(1.6);
  h_detectedPi->Draw();
  canvas2.cd(1);
  h_kaonMomentum->GetYaxis()->SetTitleOffset(1.4);
  h_kaonMomentum->Draw();
  h_cuts->SetLineColor(kRed);
  h_cuts->Draw("samel");
  canvas2.cd(2);
  h_pionMomentum->GetYaxis()->SetTitleOffset(1.4);
  h_pionMomentum->Draw();
  h_cuts->Draw("samel");
  canvas2.Print("detectionEfficiency2.pdf");
  
  TCanvas canvas3 = TCanvas("canvas3", "Detection efficiency 2",1000,1000);
  canvas3.Divide(2,2);
  canvas3.cd(1);
  h_kaonProjection->SetTitle("#DeltaLogL at 3.0 GeV;L_K - L_#pi;No. of tracks");
  h_kaonProjection->GetYaxis()->SetTitleOffset(1.5);
  h_kaonProjection->SetLineColor(kBlue);
  h_kaonProjection->Draw();
  h_pionProjection->SetLineColor(kRed);
  h_pionProjection->Draw("same");
  canvas3.cd(2);
  p_detPhotonsK->GetYaxis()->SetRangeUser(0,20);
  p_detPhotonsK->Draw();
  p_detPhotonsPi->SetLineColor(kRed);
  p_detPhotonsPi->Draw("same");
  canvas3.cd(3);
  p_lostPhotonsK->GetYaxis()->SetTitleOffset(1.4);
  p_lostPhotonsK->Draw("colz");
  canvas3.cd(4);
  p_fakePions->GetYaxis()->SetTitleOffset(1.4);
  p_fakePions->Draw("colz");
  canvas3.Print("detectionEfficiency3.pdf");
  
  TCanvas canvas4("canvas4", "Photons",1000,1000);
  canvas4.cd();
  p_detPhotons->GetYaxis()->SetTitleOffset(1.4);
  p_detPhotons->Draw("colz");
  canvas4.Print("detectionEfficiency4.pdf");
   
  return;
}

void setCuts(const TH3F& h_pion, Float_t cut[], TH1F* histogram)
{
  for (Int_t iBin=0; iBin < m_pbins; iBin++) {
    //cout << "bin " << i << endl;
      h_pion.GetXaxis()->SetRange(iBin+1,iBin+1);
      TH1F* p3 =(TH1F*) h_pion.Project3D("z");
      Double_t s3 = p3->Integral()+0.00001;
      //cout << "Integral " << s3 << endl;
      Double_t v3 = 0;
      Int_t k=0;
      for (k=m_lbins; k>0; k-- ){
	      v3 += ( p3->GetBinContent(k)/s3 );
	      if (v3 > m_lkpcut ) break;
        }
      if (k > 0) {
        cut[iBin] = p3->GetBinCenter(k) - 0.5 * p3->GetBinWidth(k) * ( v3-m_lkpcut )/( p3->GetBinContent(k)/s3);    
      } else {
      cut[iBin]= (m_lmax+m_lmin)/2.0;
      }
      histogram->AddBinContent(iBin, cut[iBin]);
      //cout << " Likelihood cut: " << cut[i][f] << endl;
    //}
  }
  return;
}
