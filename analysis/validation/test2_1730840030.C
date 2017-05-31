/*
<header>
<input>../1730840030.ntup.root</input>
<output>1730840030_Validation.root</output>
<contact>Suxian Li; lisuxian@buaa.edu.cn</contact>
</header>
*/

//////////////////////////////////////////////////////////
//
// test2_1730840030.C
// Check
//
// Constributor: Suxian Li
// May 30, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */
void plot_1Smumu(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString muidCut("Upsilon3S_Upsilon_mu0_PIDmu > 0.01 && Upsilon3S_Upsilon_mu1_PIDmu");
   TString piidCut("Upsilon3S_pi0_PIDpi > 0.01 && Upsilon3S_pi1_PIDpi > 0.01");
   TString ver_fitCut("Upsilon3S_VtxPvalue > 0.001 && Upsilon3S_Upsilon_VtxPvalue > 0.001") ;
   TString Cuts(muidCut+"&&"+ver_fitCut+"&&"+piidCut);

   TH1F *h_M_Y1S_mumu = new TH1F ("h_M_Y1S_mumu","M(1S_mumu))",100,9.36,9.56);
   ptree->Project("h_M_Y1S_mumu","Upsilon3S_Upsilon_M ",Cuts);
   h_M_Y1S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");

   TH1F *h_M_Y3S_1Smumu = new TH1F ("h_M_Y3S_1Smumu","M(Y3S_1Smumu)",100,10.2552,10.4552);
   ptree->Project("h_M_Y3S_1Smumu","Upsilon3S_M",Cuts);
   h_M_Y3S_1Smumu->GetXaxis()->SetTitle("M_{#pi^{+}#pi^{-}#Upsilon(1S)} (GeV/c^{2})");

   outputFile->cd();

   h_M_Y1S_mumu->Write();
   h_M_Y3S_1Smumu->Write();

}

void plot_1See(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString eidCut("Upsilon3S_Upsilon_e0_PIDe > 0.01 && Upsilon3S_Upsilon_e1_PIDe");
   TString piidCut("Upsilon3S_pi0_PIDpi > 0.01 && Upsilon3S_pi1_PIDpi > 0.01");
   TString ver_fitCut("Upsilon3S_VtxPvalue > 0.001 && Upsilon3S_Upsilon_VtxPvalue > 0.001") ;
   TString Cuts(eidCut+"&&"+ver_fitCut+"&&"+piidCut);

   TH1F* h_M_Y1S_ee = new TH1F("h_M_Y1S_ee","M(Y1S_ee)",100,9.36,9.56);
   ptree->Project("h_M_Y1S_ee","Upsilon3S_Upsilon_M",Cuts);
   h_M_Y1S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (GeV/c^{2})");

   TH1F* h_M_Y3S_1See = new TH1F ("h_M_Y3S_1See","M(Y3S_1See)",100,10.2552,10.4552);
   ptree->Project("h_M_Y3S_1See","Upsilon3S_M",Cuts);
   h_M_Y3S_1See->GetXaxis()->SetTitle("M_{#pi^{+}#pi^{-}#Upsilon(1S)} (GeV/c^{2})");

   outputFile->cd();

   h_M_Y1S_ee->Write();
   h_M_Y3S_1See->Write();
}

void plot_2Smumu(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString muidCut("Upsilon3S_Upsilon2S_mu0_PIDmu > 0.01 && Upsilon3S_Upsilon2S_mu1_PIDmu > 0.01");
   TString piidCut("Upsilon3S_pi0_PIDpi > 0.01 && Upsilon3S_pi1_PIDpi > 0.01");
   TString ver_fitCut("Upsilon3S_VtxPvalue > 0.001 && Upsilon3S_Upsilon2S_VtxPvalue > 0.001");
   TString Cuts(muidCut+"&&"+piidCut+"&&"+ver_fitCut);

   TH1F* h_M_Y2S_mumu = new TH1F("h_M_Y2S_mumu","M(Y2S_mumu)",100,9.9232,10.1232);
   ptree->Project("h_M_Y2S_mumu","Upsilon3S_Upsilon2S_M",Cuts);
   h_M_Y2S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");

   TH1F* h_M_Y3S_2Smumu = new TH1F ("h_M_Y3S_2Smumu","M(Y3S_2Smumu)",100,10.2552,10.4552);
   ptree->Project("h_M_Y3S_2Smumu","Upsilon3S_M",Cuts);
   h_M_Y3S_2Smumu->GetXaxis()->SetTitle("M_{#pi^{+}#pi^{-}#Upsilon(2S)} (GeV/c^{2})");

   outputFile->cd();

   h_M_Y2S_mumu->Write();
   h_M_Y3S_2Smumu->Write();

}

void plot_2See(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString eidCut("Upsilon3S_Upsilon2S_e0_PIDe > 0.01 && Upsilon3S_Upsilon2S_e1_PIDe > 0.01 ");
   TString piidCut("Upsilon3S_pi0_PIDpi > 0.01 && Upsilon3S_pi1_PIDpi > 0.01");
   TString ver_fitCut("Upsilon3S_VtxPvalue > 0.001 && Upsilon3S_Upsilon2S_VtxPvalue > 0.001");
   TString Cuts(eidCut+"&&"+piidCut+"&&"+ver_fitCut);

   TH1F* h_M_Y2S_ee = new TH1F("h_M_Y2S_ee","M(Y2S_ee)",100,9.9232,10.1232);
   ptree->Project("h_M_Y2S_ee","Upsilon3S_Upsilon2S_M",Cuts);
   h_M_Y2S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (GeV/c^{2})");

   TH1F* h_M_Y3S_2See = new TH1F ("h_M_Y3S_2See","M(Y3S_2See)",100,10.2552,10.4552);
   ptree->Project("h_M_Y3S_2See","Upsilon3S_M",Cuts);
   h_M_Y3S_2See->GetXaxis()->SetTitle("M_{#pi^{+}#pi^{-}#Upsilon(2S)} (GeV/c^{2})");

   outputFile->cd();

   h_M_Y2S_ee->Write();
   h_M_Y3S_2See->Write();
}

void plot_Mrecoil(TFile* pfile,TTree *ptree, TFile* outputFile){

   TString piidCut(" Upsilon3S_pi0_PIDpi > 0.01 &&  Upsilon3S_pi1_PIDpi > 0.01");

   TH1F* h_MRecoil_pipi = new TH1F("h_MRecoil_pipi","M(pipi)_recoil",500,9.3,10.3);
   ptree->Project("h_MRecoil_pipi","Upsilon3S_Mrecoil",piidCut);

   outputFile->cd();

   h_MRecoil_pipi->Write();

}
void test2_1730840030(){

   TString inputfile("../1730840030.ntup.root");

   TFile* sample = new TFile(inputfile);
   TTree* tree1 = (TTree*)sample->Get("ntuple_1smumu");
   TTree* tree2 = (TTree*)sample->Get("ntuple_1see");
   TTree* tree3 = (TTree*)sample->Get("ntuple_2smumu");
   TTree* tree4 = (TTree*)sample->Get("ntuple_2see");
   TTree* tree5 = (TTree*)sample->Get("ntuple_pipi");

   TFile* outputFile = new TFile("1730840030_Validation.root","RECREATE");

   plot_1Smumu(sample,tree1,outputFile);
   plot_1See(sample,tree2,outputFile);
   plot_2Smumu(sample,tree3,outputFile);
   plot_2See(sample,tree4,outputFile);
   plot_Mrecoil(sample,tree5,outputFile);

   outputFile->Close();
}
