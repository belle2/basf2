
/*
  <header>
  <input>../1730822030.ntup.root</input>
  <output>1730822030_Validation.root</output>
  <contact>Suxian Li; lisuxian@buaa.edu.cn</contact>
  </header>
  */

////////////////////////////////////////////////////////
//
// test2_1730822030.C
// Check InvM of Y(1S,2S),chi_b(2P) and efficiency.
//
// Constributor: Suxian Li
// May 30, 2017
//
/////////////////////////////////////////////////////////////////
//


#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include <TROOT.h>

void plot_1Smumu(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString pidCut1 ("Upsilon3S_chi_b12P_Upsilon_mu0_PIDmu > 0.1 && Upsilon3S_chi_b12P_Upsilon_mu1_PIDmu >0.1 ");
   
   TH1F *h_M_Y1S_mumu = new TH1F ("h_M_Y1S_mumu","M[Y(1S)->mumu]",20,9.36,9.56);
   ptree->Project("h_M_Y1S_mumu","Upsilon3S_chi_b12P_Upsilon_M",pidCut1);
   h_M_Y1S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");
   h_M_Y1S_mumu->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_Y1S_mumu->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(1S)->mu+ mu-]]"));
   h_M_Y1S_mumu->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));   
   h_M_Y1S_mumu->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of Y(1S) reconstructed by dipion"));

   TH1F *h_M_chi_b12P_1Smumu = new TH1F("h_M_chi_b12P_1Smumu","M[chi_b(2P)->1Smumu]",14,10.18,10.32);
   ptree->Project("h_M_chi_b12P_1Smumu","Upsilon3S_chi_b12P_M-Upsilon3S_chi_b12P_Upsilon_M+9.460",pidCut1);
   h_M_chi_b12P_1Smumu->GetXaxis()->SetTitle("M_{#gamma#Upsilon(1S)} (Gev/c^{2})");
   h_M_chi_b12P_1Smumu->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_chi_b12P_1Smumu->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(1S)->mu+ mu-]]"));
   h_M_chi_b12P_1Smumu->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_chi_b12P_1Smumu->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of chi_b2P(->gammma Y(1S))"));


   outputFile->cd();
   
   h_M_Y1S_mumu->Write();
   h_M_chi_b12P_1Smumu->Write();
 
}

void plot_1See(TFile* pfile, TTree* ptree, TFile* outputFile){
   
   TString pidCut2 ("Upsilon3S_chi_b12P_Upsilon_e0_PIDe >0.1 && Upsilon3S_chi_b12P_Upsilon_e1_PIDe > 0.1");
  
   TH1F* h_M_Y1S_ee = new TH1F("h_M_Y1S_ee","M[Y(1S)->ee]",20,9.36,9.56);
   ptree->Project("h_M_Y1S_ee","Upsilon3S_chi_b12P_Upsilon_M",pidCut2);
   h_M_Y1S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (Gev/c^{2})");
   h_M_Y1S_ee->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_Y1S_ee->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(1S)->e+ e-]]"));
   h_M_Y1S_ee->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_Y1S_ee->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of Y(1S)[-> e+ e-]"));
   
   TH1F* h_M_chi_b12P_1See = new TH1F("h_M_chi_b12P_1See","M[chi_b(2P)->1See]",14,10.18,10.32);
   ptree->Project("h_M_chi_b12P_1See","Upsilon3S_chi_b12P_M-Upsilon3S_chi_b12P_Upsilon_M+9.46",pidCut2);
   h_M_chi_b12P_1See->GetXaxis()->SetTitle("M_{#gamma#Upsilon(1S)} (Gev/c^{2})");
   h_M_chi_b12P_1See->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_chi_b12P_1See->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(1S)->e+ e-]]"));
   h_M_chi_b12P_1See->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_chi_b12P_1See->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of chi_b2P(->gammma Y(1S)) "));
   
   outputFile->cd();
    
   h_M_Y1S_ee->Write();
   h_M_chi_b12P_1See->Write();

}

void plot_2Smumu(TFile* pfile,TTree *ptree, TFile* outputFile){

   TString pidCut3 ("Upsilon3S_chi_b12P_Upsilon2S_mu0_PIDmu > 0.1 && Upsilon3S_chi_b12P_Upsilon2S_mu1_PIDmu >0.1");
  
   TH1F* h_M_Y2S_mumu = new TH1F("h_M_Y2S_mumu","M[Y(2S)->mumu]",20,9.9232,10.1232);
   ptree->Project("h_M_Y2S_mumu","Upsilon3S_chi_b12P_Upsilon2S_M",pidCut3);
   h_M_Y2S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");
   h_M_Y2S_mumu->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_Y2S_mumu->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(2S)->mu+ mu-]]"));
   h_M_Y2S_mumu->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_Y2S_mumu->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of Y(2S)[->mu+ mu-]"));

   TH1F* h_M_chi_b12P_2Smumu = new TH1F("h_M_chi_b12P_2Smumu","M[chi_b(2P)->2Smumu]",14,10.18,10.32);
   ptree->Project("h_M_chi_b12P_2Smumu","Upsilon3S_chi_b12P_M-Upsilon3S_chi_b12P_Upsilon2S_M+10.032",pidCut3);
   h_M_chi_b12P_2Smumu->GetXaxis()->SetTitle("M_{#gamma#Upsilon(2S)} (Gev/c^{2})");
   h_M_chi_b12P_2Smumu->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_chi_b12P_2Smumu->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(2S)->mu+ mu-]]"));
   h_M_chi_b12P_2Smumu->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_chi_b12P_2Smumu->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of chi_b2P(->gammma mu+ mu-) and efficiency=N(candidate)/N(totol[calculated through genereting events and branch ratio])"));


   outputFile->cd();

   h_M_Y2S_mumu->Write();
   h_M_chi_b12P_2Smumu->Write();

}

void plot_2See(TFile* pfile,TTree *ptree, TFile* outputFile){

   TString pidCut4 ("Upsilon3S_chi_b12P_Upsilon2S_e0_PIDe > 0.1 && Upsilon3S_chi_b12P_Upsilon2S_e1_PIDe > 0.1");
   
   TH1F* h_M_Y2S_ee = new TH1F("h_M_Y2S_ee","M[Y(2S)->ee]",20,9.9232,10.1232);
   ptree->Project("h_M_Y2S_ee","Upsilon3S_chi_b12P_Upsilon2S_M",pidCut4);
   h_M_Y2S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (Gev/c^{2})");
   h_M_Y2S_ee->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_Y2S_ee->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(2S)->e+ e-]]"));
   h_M_Y2S_ee->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_Y2S_ee->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of Y(2S)[->e+ e-]"));

   TH1F* h_M_chi_b12P_2See = new TH1F("h_M_chi_b12P_2See","M[chi_b(2P)->2See]",14,10.18,10.32);
   ptree->Project("h_M_chi_b12P_2See","Upsilon3S_chi_b12P_M-Upsilon3S_chi_b12P_Upsilon2S_M+10.023",pidCut4);
   h_M_chi_b12P_2See->GetXaxis()->SetTitle("M_{#gamma#Upsilon(2S)} (Gev/c^{2})");
   h_M_chi_b12P_2See->GetYaxis()->SetTitle("Event /0.01GeV/c^{2}");
   h_M_chi_b12P_2See->GetListOfFunctions()->Add(new TNamed("Description","Y(3S)->gamma [chi_b2P->gamma [Y(2S)->e+ e-]]"));
   h_M_chi_b12P_2See->GetListOfFunctions()->Add(new TNamed("Contact","lisuxian@buaa.edu.cn"));
   h_M_chi_b12P_2See->GetListOfFunctions()->Add(new TNamed("Check","check the InvM of chi_b2P(->gammma e+ e-)"));

   outputFile->cd();

   h_M_Y2S_ee->Write();
   h_M_chi_b12P_2See->Write();

}
void test2_1730822030(){

   TString inputfile("../1730822030.ntup.root");

   TFile* sample = new TFile(inputfile);
   TTree* tree1 = (TTree*)sample->Get("ntuple_1smumu");
   TTree* tree2 = (TTree*)sample->Get("ntuple_1see");  
   TTree* tree3 = (TTree*)sample->Get("ntuple_2smumu"); 
   TTree* tree4 = (TTree*)sample->Get("ntuple_2see");
 
   TFile* outputFile = new TFile("1730822030_Validation.root","RECREATE");
   
   plot_1Smumu(sample,tree1,outputFile);
   plot_1See(sample,tree2,outputFile);
   plot_2Smumu(sample,tree3,outputFile);   
   plot_2See(sample,tree4,outputFile);
  
   outputFile->Close();



}

