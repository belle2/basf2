/*
<header>
  <input>../3470023000.dst.root</input>
  <output>../3470023000.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////////
//
// Analysis Code (make Ntuple for validation of tau->a0(->eta pi)nu)
// contributor : Maeda Yosuke (KMI, Nagoya Univ.)
//               maeday@hepl.phys.nagoya-u.ac.jp  (March 2018)
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <string>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TString.h>
#include <TCut.h>
#include <TMath.h>

void plot_eta3pi( TTree* tree, TFile* outputFile ){

  TCut etaMassCut( "TMath::Abs(Z0_a_0_eta_M-0.5475)<0.0125" );
  TCut piMomCut( "TMath::Hypot(Z0_a_0_pi_P4[0],Z0_a_0_pi_P[1])>0.3 && TMath::Hypot(Z0_a_0_eta_pi1_P4[0],Z0_a_0_eta_pi1_P4[1])>0.3 && TMath::Hypot(Z0_a_0_eta_pi2_P4[0],Z0_a_0_eta_pi2_P4[1])>0.3" );
  TCut gammaECut( "Z0_a_0_eta_pi0_gamma0_P>0.1 && Z0_a_0_eta_pi0_gamma1_P>0.1" );
  TCut leptonMomCut( "TMath::Hypot(Z0_pi_P4[0],Z0_pi_P4[1])>0.3" );
  TCut isSignal( "Z0_a_0_isSignal>0.5 && (TMath::Abs(Z0_pi_mcPDG)==11 || TMath::Abs(Z0_pi_mcPDG)==13)" );//to reduce multiple counting of each single event
  TCut a0MassCut( "TMath::Abs(Z0_a_0_M-1.0)<0.2" );

  TH1F* hMEta = new TH1F( "hMEta_3pi", "reconstructed mass of #eta#rightarrow#pi^{+}#pi^{-}#pi^{0}",
			  100, 0.45, 0.65 );
  tree->Project( hMEta->GetName(), "Z0_a_0_eta_M",
		 piMomCut + gammaECut + leptonMomCut + isSignal );
  hMEta->GetXaxis()->SetTitle( "#it{M}_{#eta#rightarrow#pi^{+}#pi^{-}#pi^{0}} [GeV/#it{c}^{2}]" );
  hMEta->GetYaxis()->SetTitle( "Entries [/(0.002 GeV/#it{c}^{2})]" );

  TH1F* hMA0 = new TH1F( "hMA0_3pi", "reconstructed mass of #it{a}_{0}#rightarrow#eta#pi^{-} (#eta#rightarrow#pi^{+}#pi^{-}#pi^{0})",
			100, 0.5, 1.5 );
  tree->Project( hMA0->GetName(), "Z0_a_0_M",
		 etaMassCut + piMomCut + gammaECut + leptonMomCut + isSignal );
  hMA0->GetXaxis()->SetTitle( "#it{M}_{#it{a}_{0}} [GeV/#it{c}^{2}]" );
  hMA0->GetYaxis()->SetTitle( "Entries [/(0.01 GeV/#it{c}^{2})]" );
  std::cout << "eta->pi+pi-pi0 :: entries for 0.8-1.2 GeV/c2 = " << hMA0->Integral(hMA0->GetXaxis()->FindBin(0.801),hMA0->GetXaxis()->FindBin(1.199))
	    << std::endl;

  Int_t nEntries = tree->Draw( "evt_no", etaMassCut + piMomCut + gammaECut + leptonMomCut + isSignal + a0MassCut );
  Int_t nMultiCands = 0;
  Int_t multiplicity = 0;
  Int_t maxMultiplicity = 0;
  for( Int_t iEntry=0 ; iEntry<nEntries-1 ; iEntry++ ){
    if( TMath::Abs(tree->GetV1()[iEntry]-tree->GetV1()[iEntry+1])<0.1 ){
      nMultiCands++;
      multiplicity++;
      if( multiplicity>maxMultiplicity ) maxMultiplicity = multiplicity; 
    }else{
      multiplicity = 0;
    } 
  }
  std::cout << "multi counting : " << nMultiCands << " / " << nEntries
	    << ", max # of candidates = " << (maxMultiplicity+1) << std::endl;
  
  outputFile->cd();
  hMEta->Write();
  hMA0->Write();
  
  return;
}

void plot_etagg( TTree* tree, TFile* outputFile ){

  TCut etaMassCut( "TMath::Abs(Z0_a_0_eta_M-0.545)<0.045" );
  TCut piMomCut( "TMath::Hypot(Z0_a_0_pi_P4[0],Z0_a_0_pi_P4[1])>0.3" );
  TCut gammaECut( "Z0_a_0_eta_gamma0_P>0.3 && Z0_a_0_eta_gamma1_P>0.3" );
  TCut leptonMomCut( "TMath::Hypot(Z0_pi_P4[0],Z0_pi_P4[1])>0.3" );
  TCut isSignal( "Z0_a_0_isSignal>0.5 && (TMath::Abs(Z0_pi_mcPDG)==11 || TMath::Abs(Z0_pi_mcPDG)==13)" );//to reduce multiple counting of each single event
  TCut a0MassCut( "TMath::Abs(Z0_a_0_M-1.0)<0.2" );  

  TH1F* hMEta = new TH1F( "hMEta_gg", "reconstructed mass of #eta#rightarrow#gamma#gamma",
			  100, 0.30, 0.80 );
  tree->Project( hMEta->GetName(), "Z0_a_0_eta_M",
		 piMomCut + gammaECut + leptonMomCut + isSignal );
  hMEta->GetXaxis()->SetTitle( "#it{M}_{#eta#rightarrow#gamma#gamma} [GeV/#it{c}^{2}]" );
  hMEta->GetYaxis()->SetTitle( "Entries [/(0.005 GeV/#it{c}^{2})]" );
  
  TH1F* hMA0 = new TH1F( "hMA0_gg", "reconstructed mass of #it{a}_{0}#rightarrow#eta#pi^{-} (#eta#rightarrow#gamma#gamma)",
			 100, 0.5, 1.5 );
  tree->Project( hMA0->GetName(), "Z0_a_0_M",
		 etaMassCut + piMomCut + gammaECut + leptonMomCut + isSignal );
  hMA0->GetXaxis()->SetTitle( "#it{M}_{#it{a}_{0}} [GeV/#it{c}^{2}]" );
  hMA0->GetYaxis()->SetTitle( "Entries [/(0.01 GeV/#it{c}^{2})]" );
  std::cout << "eta->gamma gamma :: entries for 0.8-1.2 GeV/c2 = " << hMA0->Integral(hMA0->GetXaxis()->FindBin(0.801),hMA0->GetXaxis()->FindBin(1.199))
	    << std::endl;
  
  Int_t nEntries = tree->Draw( "evt_no", etaMassCut + piMomCut + gammaECut + leptonMomCut + isSignal + a0MassCut );
  Int_t nMultiCands = 0;
  Int_t multiplicity = 0;
  Int_t maxMultiplicity = 0;
  for( Int_t iEntry=0 ; iEntry<nEntries-1 ; iEntry++ ){
    if( TMath::Abs(tree->GetV1()[iEntry]-tree->GetV1()[iEntry+1])<0.1 ){
      nMultiCands++;
      multiplicity++;
      if( multiplicity>maxMultiplicity ) maxMultiplicity = multiplicity; 
    }else{
      multiplicity = 0;
    } 
  }
  std::cout << "multi counting : " << nMultiCands << " / " << nEntries
	    << ", max # of candidates = " << (maxMultiplicity+1) << std::endl;

  outputFile->cd();
  hMEta->Write();
  hMA0->Write();
  
  return;
}

void test2_3470023000(){

  TString inputfile("../3470023000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree_3pi = (TTree*)sample->Get("tautau_3pi");
  TTree* tree_gg = (TTree*)sample->Get("tautau_gg");

  TFile* outputFile = new TFile("3470023000_Validation.root","RECREATE");
  
  plot_eta3pi( tree_3pi, outputFile );
  plot_etagg( tree_gg, outputFile );
  outputFile->Close();
    
  return;
}
