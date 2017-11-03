/*
<header>
<input>../1213030101.ntup.root</input>
<output>1213030101_Validation.root</output>
<contact>Minakshi Nayak; minakshi.nayak@wayne.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1213030101.C
// Check Mbc, DeltaE, MD and MKs
//
// Constributor: Minakshi Nayak
// May 17, 2017
// modified October 31, 2017
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

 /* Draw the validation plots */

void plot(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString tmCuts("(B_isSignal == 1)");

  TH1F* h_mbc = new TH1F("h_mbc","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}",100,5.25,5.29);
  //  ptree->Project("h_mbc", "B_mbc", tmCuts);
  ptree->Project("h_mbc", "B_mbc");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_mbc->GetYaxis()->SetTitle("Evts/bin");
  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}  M_{bc} [GeV/c^{2}]"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "mass is expected to peak at 5.279 GeV/c^{2} with resolution around 3 MeV"));



  TH1F* h_deltae = new TH1F("h_deltae","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}",100,-0.1,0.1);
  //  ptree->Project("h_deltae", "B_deltae", tmCuts);
  ptree->Project("h_deltae", "B_deltae");
  h_deltae->GetXaxis()->SetTitle("#Delta E (GeV)");
  h_deltae->GetYaxis()->SetTitle("Evts/bin");
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+} #DeltaE [GeV]"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "#Delta E is expected to peak at 0 GeV with resolution around 17 MeV"));


  TH1F* h_MKs = new TH1F("h_MKs","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}",100,0.45,0.55);
  //  ptree->Project("h_MKs", "B_D0_K_S0_M", tmCuts);
  ptree->Project("h_MKs", "B_D0_K_S0_M");
  h_MKs->GetXaxis()->SetTitle("M_{Ks} (GeV/c^{2})");
  h_MKs->GetYaxis()->SetTitle("Evts/bin");
  h_MKs->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}  M_{K_{S}} (GeV/c^{2})"));
  h_MKs->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_MKs->GetListOfFunctions()->Add(new TNamed("Check", "M_{K_{S}} is expected to peak at 497 MeV/c^{2} with resolution around 4 MeV"));


  TH1F* h_MD = new TH1F("h_MD","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}",100,1.8,1.9);
  //  ptree->Project("h_MD", "B_D0_M", tmCuts);
  ptree->Project("h_MD", "B_D0_M");
  h_MD->GetXaxis()->SetTitle("M_{D} (GeV/c^{2})");
  h_MD->GetYaxis()->SetTitle("Evts/bin");
  h_MD->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{+} #pi^{-})K^{+}  M_{D} (GeV/c^{2})"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Check", "M_{D} is expected to peak at 1.864 GeV/c^{2} with resolution around 8 MeV"));


  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_MKs->Write();
  h_MD->Write(); 

}




void test2_1213030101(){

  TString inputfile("../1213030101.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("bdk");

  TFile* outputFile = new TFile("1213030101_Validation.root","RECREATE");

  plot(sample, tree, outputFile);


  outputFile->Close();

}
