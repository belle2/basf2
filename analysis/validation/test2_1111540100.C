/*
 <header>
 <input>../1111540100.ntup.root</input>
 <output>1111540100_Validation.root</output>
 <contact>A. Mordà ; morda@pd.infn.it</contact>
 </header>
 */

//////////////////////////////////////////////////////////
//
// test2_1111540100.C
// Check the Delta E and Mbc distributions for
//  B0->J/psi(ee)K0S(pi+pi-)
// Constributor: A. Mordà 
// May 25, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include <TF1.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TSystem.h>

/* Validation script to determine benchmarks for PID cuts. */

void plot_dembc(TFile* pfile, TTree* ptree, TFile *outputFile){
  
 TString tmCuts("B0_isSignal");
 TString title = "B^{0}#rightarrow J/#psi(ee) K_{S}^{0}";
 TH1F* h_mbc = new TH1F("h_mbc",title,180,5.2,5.29);
 ptree->Project("h_mbc", "B0_mbc", tmCuts);
 h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");  
 h_mbc->GetYaxis()->SetTitle("Evts/bin");                     
 h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "B^{0}#rightarrow J/#psi(ee) K_{S}^{0}  M_{bc} [GeV/c^{2}]"));
 h_mbc->GetListOfFunctions()->Add(new TNamed("Contact" , "morda@pd.infn.it"));
 h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "mass is expected to peak at 5.279 GeV/c^{2}"));
 gStyle->SetStatX(0.1);
 
 TH1F* h_deltae = new TH1F("h_deltae",title,100,-0.5,0.5);
 ptree->Project("h_deltae", "B0_deltae", tmCuts);
 h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)"); 
 h_deltae->GetYaxis()->SetTitle("Evts/bin");           
 h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "B^{0}#rightarrow J/#psi(ee)  K_{S}^{0}  #DeltaE [GeV]"));
 h_deltae->GetListOfFunctions()->Add(new TNamed("Contact" , "morda@pd.infn.it"));
 h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "#Delta E is expected to peak at 0 GeV"));

 TH1F* h_Tres = new TH1F("h_Tres",title,100,-10,10);
 ptree->Project("h_Tres", "B0_DeltaT-B0_TruthDeltaT", tmCuts);
 h_Tres->GetXaxis()->SetTitle("#delta(#Delta T) [ps]"); 
 h_Tres->GetYaxis()->SetTitle("Evts/bin");           
 h_Tres->GetListOfFunctions()->Add(new TNamed("Description", "B^{0}#rightarrow J/#psi(ee) K_{S}^{0}  #delta(#Delta T)#equiv B0_DeltaT-B0_TruthDeltaT [ps]"));
 h_Tres->GetListOfFunctions()->Add(new TNamed("Contact" , "morda@pd.infn.it"));
 h_Tres->GetListOfFunctions()->Add(new TNamed("Check", "Time resolution has to peak at 0, the smaller the width of the distribution the better is"));

 TH1F* h_eff = new TH1F("h_eff",title,1,0,1);
 h_eff->SetBinContent(1,ptree->GetEntries("B0_isSignal")/1000.0);
 h_eff->GetYaxis()->SetTitle("#varepsilon");           
 h_eff->GetListOfFunctions()->Add(new TNamed("Description", "B^{0}#rightarrow J/#psi(ee) K_{S}^{0} #varepsilon #equiv #frac{true selected signal events}{total number of events}"));
 h_eff->GetListOfFunctions()->Add(new TNamed("Contact" , "morda@pd.infn.it"));
 h_eff->GetListOfFunctions()->Add(new TNamed("Check", "Efficiency has to be the greatest as possible"));
 
 outputFile->cd();

 h_mbc->Write();
 h_deltae->Write();
 h_Tres->Write();
 h_eff->Write();
}

void test2_1111540100(){

 TString inputfile("../1111540100.ntup.root");

 TFile* f = new TFile(inputfile,"READ");
  if(!f) {
    std::cerr << "Couldn't read file!\n";
    exit(1);
  }
 TTree *tree = (TTree*)f->Get("B0");
  if(!tree) {
    std::cerr << "Couldn't find 'tree'!\n";
    exit(1);
  }
  if(tree->GetEntries() == 0) {
    std::cerr << "Input file is empty, aborting!\n";
    exit(1);
  }
 TFile* outputFile = new TFile("1111540100_Validation.root","RECREATE");

 plot_dembc(f, tree, outputFile);
 outputFile->Close();

}
