/*
<header>
<input>../1213030000.ntup.root</input>
<output>1213030000_Validation.root</output>
<contact>Minakshi Nayak; minakshi.nayak@wayne.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1213030000.C
// Check Mbc, DeltaE, MD
//
// Constributor: Minakshi Nayak
// November 05, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TApplication.h"
#include "TGraph.h"
#include <TROOT.h>
#include <TSystem.h>

/* Validation script to determine benchmarks for PID cuts. */

void plot(TFile* pfile, TTree* ptree, TFile *outputFile){


  TString tmCuts("(B_isSignal == 1)");

  TH1F* h_mbc = new TH1F("h_mbc","B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}",100,5.2,5.29);
  //  ptree->Project("h_mbc", "B_mbc", tmCuts);
  ptree->Project("h_mbc", "B_mbc");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_mbc->GetYaxis()->SetTitle("Evts/bin");
  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}  M_{bc} [GeV/c^{2}]"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "mass is expected to peak at 5.279 GeV/c^{2} with resolution around 10 MeV"));

  TH1F* h_deltae = new TH1F("h_deltae","B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}",100,-0.3,0.3);
  //  ptree->Project("h_deltae", "B_deltae", tmCuts);
  ptree->Project("h_deltae", "B_deltae");
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");
  h_deltae->GetYaxis()->SetTitle("Evts/bin");
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}  #DeltaE [GeV]"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "#Delta E is expected to peak at 0 GeV with resolution around 50 MeV"));


  TH1F* h_MD = new TH1F("h_MD","B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}",100,1.83,1.90);
  //  ptree->Project("h_MD", "B_D0_M", tmCuts);
  ptree->Project("h_MD", "B_D0_M");
  h_MD->GetXaxis()->SetTitle("M_{D} (GeV/c^{2})");
  h_MD->GetYaxis()->SetTitle("Evts/bin");
  h_MD->GetListOfFunctions()->Add(new TNamed("Description", "B^{-}#rightarrow D^{0}(K^{-} #pi^{+})#pi^{-}  M_{D} (GeV/c^{2})"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Check", "M_{D} is expected to peak at 1.864 GeV/c^{2} with resolution around 10 MeV"));



  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_MD->Write();
}




void test2_1213030000(){

  TString inputfile("../1213030000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("bdpi");

  TFile* outputFile = new TFile("1213030000_Validation.root","RECREATE");

  plot(sample, tree, outputFile);


  outputFile->Close();

}
