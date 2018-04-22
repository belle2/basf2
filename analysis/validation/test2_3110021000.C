/*
<header>
  <input>../3110021000.dst.root</input>
  <output>../3110021000.ntup.root</output>
</header>
*/

////////////////////////////////////////////////////////////////
//
// Analysis Code (make Ntuple for validation of ee->pipigamma)
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

void plot( TTree* tree, TFile* outputFile ){

  TCut gammaDirCut( "TMath::Abs(Z0_gamma_P4[2]/Z0_gamma_P-0.123)<0.696" );
  TCut pionDirCut( "TMath::Abs(Z0_rho0_pi0_P4[2]/Z0_rho0_pi0_P-0.132)<0.774 && TMath::Abs(Z0_rho0_pi1_P4[2]/Z0_rho0_pi1_P-0.132)<0.774" );
  TCut pidCut_1( "Z0_rho0_pi0_electronID<0.9 && Z0_rho0_pi0_muonID<0.9 && Z0_rho0_pi0_kaonID<0.9" );
  TCut pidCut_2( "Z0_rho0_pi1_electronID<0.9 && Z0_rho0_pi1_muonID<0.9 && Z0_rho0_pi1_kaonID<0.9" );
  TCut MpipigammaCut( "TMath::Abs(Z0_M-10.5)<0.5" );
  
  TH1F* hGammaDir = new TH1F( "hGammaDir", "cosine of ISR #gamma #theta angle in the lab frame", 100, -1, 1 );
  tree->Project( hGammaDir->GetName(), "Z0_gamma_P4[2]/Z0_gamma_P",
		 pionDirCut + pidCut_1 + pidCut_2 + MpipigammaCut );
  hGammaDir->GetXaxis()->SetTitle( "cos#theta_{#gamma}" );
  hGammaDir->GetYaxis()->SetTitle( "Entries [/0.02]" );
  hGammaDir->GetListOfFunctions()->Add(new TNamed("Description", hGammaDir->GetTitle()));
  hGammaDir->GetListOfFunctions()->Add(new TNamed("Contact", "Maeda Yosuke; maeday@hepl.phys.nagoya-u.ac.jp"));
  hGammaDir->GetListOfFunctions()->Add(new TNamed("Check", "a gap at -0.65 and a peak at 0.8"));

  TH1F* hMpipi = new TH1F( "hMpipi", "reconstructed mass of #pi^{+}#pi^{-}", 100, 0.2, 2.2 );
  tree->Project( hMpipi->GetName(), "Z0_rho0_M",
		 gammaDirCut + pionDirCut + pidCut_1 + pidCut_2 + MpipigammaCut );
  hMpipi->GetXaxis()->SetTitle( "#it{M}_{#pi^{+}#pi^{-}} [GeV/#it{c}^{2}]" );
  hMpipi->GetYaxis()->SetTitle( "Entries [/(0.02 GeV/#it{c}^{2})]" );
  hMpipi->GetListOfFunctions()->Add(new TNamed("Description", hMpipi->GetTitle()));
  hMpipi->GetListOfFunctions()->Add(new TNamed("Contact", "Maeda Yosuke; maeday@hepl.phys.nagoya-u.ac.jp"));
  hMpipi->GetListOfFunctions()->Add(new TNamed("Check", "you should see the #rho meason resonance; also overall shape is also important"));

  TH1F* hMpipigamma = new TH1F( "hMpipigamma", "reconstructed mass of #pi^{+}#pi^{-}#gamma", 100, 7.5, 12.5 );
  tree->Project( hMpipigamma->GetName(), "Z0_M",
		 gammaDirCut + pionDirCut + pidCut_1 + pidCut_2 );
  hMpipigamma->GetXaxis()->SetTitle( "#it{M}_{#pi^{+}#pi^{-}#gamma} [GeV/#it{c}^{2}]" );
  hMpipigamma->GetYaxis()->SetTitle( "Entries [/(0.05 GeV/#it{c}^{2})]" );
  hMpipigamma->GetListOfFunctions()->Add(new TNamed("Description", hMpipigamma->GetTitle()));
  hMpipigamma->GetListOfFunctions()->Add(new TNamed("Contact", "Maeda Yosuke; maeday@hepl.phys.nagoya-u.ac.jp"));
  hMpipigamma->GetListOfFunctions()->Add(new TNamed("Check", "a peak at the collision energy and some tail in the low energy region"));
  
  std::cout << "total # of events : " << hMpipi->GetEntries() << std::endl;
  std::cout << "  low Mpipi (<0.5 GeV/c2)      : " << hMpipi->Integral(0,15) << std::endl;
  std::cout << "  middle Mpipi (0.5-1.0 GeV/c2): " << hMpipi->Integral(16,40) << std::endl;
  std::cout << "  high Mpipi (>1.0 GeV/c2)     : " << hMpipi->Integral(41,101) << std::endl;

  Int_t nEntries = tree->Draw( "evt_no", gammaDirCut + pionDirCut + pidCut_1 + pidCut_2 );
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
  hGammaDir->Write();
  hMpipi->Write();
  hMpipigamma->Write();
  
  return;
}

void test2_3110021000(){

  TString inputfile("../3110021000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("isr");

  TFile* outputFile = new TFile("3110021000_Validation.root","RECREATE");
  
  plot( tree, outputFile );
  outputFile->Close();
    
  return;
}
