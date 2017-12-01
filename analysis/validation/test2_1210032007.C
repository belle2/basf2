/*
<header>
<input>../1210032007.ntup.root</input>
<output>1210032007_Validation.root</output>
<contact>Komarov Ilya; ilya.komarov@desy.de</contact>
<description>Distribuions used in angular analysis for 
longitudinaly-polarised B->K*(->K+pi0)rho(->pi+pi-) candidates:
  <ul>
      <li>&#x394;E</li>
      <li>Mbc</li>
      <li>M(&#x2208;(770))</li>
      <li>M(K*(892))</li>
      <li>cos(&#x3B8;(&#x3C0;&#x3C0;))</li>
      <li>cos(&#x3B8;(K&#x3C0;))</li>
  </ul>
For each variables three histograms are created:
  <ul>
      <li>Gen-level historgram, i.e. distirbuion of the variable
       for denerated candidate</li>
      <li>True value historgram, i.e. distirbuion of true (generated)
       values of the given variable for the correctly reconstructed candidates</li>
      <li>Rec-level historgram, i.e. distirbuion of reconstructed
       values of the given variable for the correctly reconstructed candidates</li>
  </ul>
</description>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1210032007.C
// Check Mbc, DeltaE, MR, MK, helK, helR
//
// Constributor: Komarov Ilya
// October 30, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script for angular analysis variables. */

void plot(TTree* ptree, TTree* pmctree, TFile *outputFile){


  TString sigCut("(B_isSignal == 1)");

  TH1F* gen_mbc = new TH1F("gen_mbc","M_{bc} (gen-level) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,5.25,5.29);
  pmctree->Project("gen_mbc", "B_Mbc");
  gen_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* true_mbc = new TH1F("true_mbc","True M_{bc} of reconstructed signals \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,5.25,5.29);
  ptree->Project("true_mbc", "B_MCT_Mbc", sigCut);
  true_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* rec_mbc = new TH1F("rec_mbc","M_{bc} of reconstructed signals \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,5.25,5.29);
  ptree->Project("rec_mbc", "B_MCT_Mbc", sigCut);
  rec_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* gen_deltae = new TH1F("gen_deltae","Generated value of #DeltaE \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-0.2,0.15);
  pmctree->Project("gen_deltae", "B_deltaE");
  gen_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* true_deltae = new TH1F("true_deltae","Generated value for reconstructed candidates: #DeltaE \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-0.2,0.15);
  ptree->Project("true_deltae", "B_MCT_deltaE", sigCut);
  true_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* rec_deltae = new TH1F("rec_deltae","#Reconstructed value of DeltaE \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-0.2,0.15);
  ptree->Project("rec_deltae", "B_deltaE", sigCut);
  rec_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* gen_MK = new TH1F("gen_MK","Generated value of M_{K*(892)^{+}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.792,0.992);
  pmctree->Project("gen_MK", "B_MK");
  gen_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* true_MK = new TH1F("true_MK","Generated value for reconstructed candidates: M_{K*(892)^{+}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.792,0.992);
  ptree->Project("true_MK", "B_MCT_MK", sigCut);
  true_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* rec_MK = new TH1F("rec_MK","Reconstructed value of M_{K*(892)^{+}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.792,0.992);
  ptree->Project("rec_MK", "B_MK", sigCut);
  rec_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* gen_MR = new TH1F("gen_MR","Generated value of M_{#rho(770)^{0}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.5,1.05);
  pmctree->Project("gen_MR", "B_MR");
  gen_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* true_MR = new TH1F("true_MR","Generated value for reconstructed candidates: M_{#rho(770)^{0}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.5,1.05);
  ptree->Project("true_MR", "B_MCT_MR", sigCut);
  true_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* rec_MR = new TH1F("rec_MR","Reconstructed value of M_{#rho(770)^{0}} \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,0.5,1.05);
  ptree->Project("rec_MR", "B_MR", sigCut);
  rec_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* gen_helK = new TH1F("gen_helK","Generated value of cos(#theta_{K^+#pi^0}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  pmctree->Project("gen_helK", "B_helK");
  gen_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* true_helK = new TH1F("true_helK","Generated value for reconstructed candidates: cos(#theta_{K^+#pi^0}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  ptree->Project("true_helK", "B_MCT_helK", sigCut);
  true_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* rec_helK = new TH1F("rec_helK","Reconstructed value of cos(#theta_{K^+#pi^0}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  ptree->Project("rec_helK", "B_helK", sigCut);
  rec_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* gen_helR = new TH1F("gen_helR","Generated value of cos(#theta_{#pi^+#pi^-}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  pmctree->Project("gen_helR", "B_helR");
  gen_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TH1F* true_helR = new TH1F("true_helR","Generated value for reconstructed candidates: cos(#theta_{#pi^+#pi^-}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  ptree->Project("true_helR", "B_MCT_helR", sigCut);
  true_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TH1F* rec_helR = new TH1F("rec_helR","Reconstructed value of cos(#theta_{#pi^+#pi^-}) \
    (for B #rightarrow K*(892)^{+}(#rightarrow K^{+} #pi^{0})\
     #rho(770)(#rightarrow #pi #pi), fL=1)",100,-1.,1.);
  ptree->Project("rec_helR", "B_helR", sigCut);
  rec_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TNamed* mo_expert_nc = new TNamed("MetaOptions", "expert, nocompare");
  TNamed* mo_nc = new TNamed("MetaOptions", "expert, nocompare");
  TNamed* contact = new TNamed("Contact", "ilya.komarov@desy.de");

  gen_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  gen_mbc->GetListOfFunctions()->Add(new TNamed("Description", gen_mbc->GetTitle()));
  gen_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  gen_mbc->GetListOfFunctions()->Add(contact);
  true_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  true_mbc->GetListOfFunctions()->Add(new TNamed("Description", true_mbc->GetTitle()));
  true_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  true_mbc->GetListOfFunctions()->Add(contact);
  rec_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  rec_mbc->GetListOfFunctions()->Add(new TNamed("Description", rec_mbc->GetTitle()));
  rec_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  rec_mbc->GetListOfFunctions()->Add(contact);
  gen_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  gen_deltae->GetListOfFunctions()->Add(new TNamed("Description", gen_deltae->GetTitle()));
  gen_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  gen_deltae->GetListOfFunctions()->Add(contact);
  true_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  true_deltae->GetListOfFunctions()->Add(new TNamed("Description", true_deltae->GetTitle()));
  true_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  true_deltae->GetListOfFunctions()->Add(contact);
  rec_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  rec_deltae->GetListOfFunctions()->Add(new TNamed("Description", rec_deltae->GetTitle()));
  rec_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  rec_deltae->GetListOfFunctions()->Add(contact);
  gen_MK->GetListOfFunctions()->Add(mo_expert_nc);
  gen_MK->GetListOfFunctions()->Add(new TNamed("Description", gen_MK->GetTitle()));
  gen_MK->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  gen_MK->GetListOfFunctions()->Add(contact);
  true_MK->GetListOfFunctions()->Add(mo_expert_nc);
  true_MK->GetListOfFunctions()->Add(new TNamed("Description", true_MK->GetTitle()));
  true_MK->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  true_MK->GetListOfFunctions()->Add(contact);
  rec_MK->GetListOfFunctions()->Add(mo_expert_nc);
  rec_MK->GetListOfFunctions()->Add(new TNamed("Description", rec_MK->GetTitle()));
  rec_MK->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  rec_MK->GetListOfFunctions()->Add(contact);
  gen_MR->GetListOfFunctions()->Add(mo_expert_nc);
  gen_MR->GetListOfFunctions()->Add(new TNamed("Description", gen_MR->GetTitle()));
  gen_MR->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  gen_MR->GetListOfFunctions()->Add(contact);
  true_MR->GetListOfFunctions()->Add(mo_expert_nc);
  true_MR->GetListOfFunctions()->Add(new TNamed("Description", true_MR->GetTitle()));
  true_MR->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  true_MR->GetListOfFunctions()->Add(contact);
  rec_MR->GetListOfFunctions()->Add(mo_expert_nc);
  rec_MR->GetListOfFunctions()->Add(new TNamed("Description", rec_MR->GetTitle()));
  rec_MR->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  rec_MR->GetListOfFunctions()->Add(contact);
  gen_helK->GetListOfFunctions()->Add(mo_expert_nc);
  gen_helK->GetListOfFunctions()->Add(new TNamed("Description", gen_helK->GetTitle()));
  gen_helK->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution, cut at ~0.8"));
  gen_helK->GetListOfFunctions()->Add(contact);
  true_helK->GetListOfFunctions()->Add(mo_expert_nc);
  true_helK->GetListOfFunctions()->Add(new TNamed("Description", true_helK->GetTitle()));
  true_helK->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution, cut at ~0.8"));
  true_helK->GetListOfFunctions()->Add(contact);
  rec_helK->GetListOfFunctions()->Add(mo_nc);
  rec_helK->GetListOfFunctions()->Add(new TNamed("Description", rec_helK->GetTitle()));
  rec_helK->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution, cut at ~0.8"));
  rec_helK->GetListOfFunctions()->Add(contact);
  gen_helR->GetListOfFunctions()->Add(mo_expert_nc);
  gen_helR->GetListOfFunctions()->Add(new TNamed("Description", gen_helR->GetTitle()));
  gen_helR->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution"));
  gen_helR->GetListOfFunctions()->Add(contact);
  true_helR->GetListOfFunctions()->Add(mo_expert_nc);
  true_helR->GetListOfFunctions()->Add(new TNamed("Description", true_helR->GetTitle()));
  true_helR->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution"));
  true_helR->GetListOfFunctions()->Add(contact);
  rec_helR->GetListOfFunctions()->Add(mo_nc);
  rec_helR->GetListOfFunctions()->Add(new TNamed("Description", rec_helR->GetTitle()));
  rec_helR->GetListOfFunctions()->Add(new TNamed("Check", "'x^2'-like distribution"));
  rec_helR->GetListOfFunctions()->Add(contact);
  outputFile->cd();
  gen_mbc->Write();
  true_mbc->Write();
  rec_mbc->Write();
  gen_deltae->Write();
  true_deltae->Write();
  rec_deltae->Write();
  gen_MK->Write();
  true_MK->Write();
  rec_MK->Write();
  gen_MR->Write();
  true_MR->Write();
  rec_MR->Write();
  gen_helK->Write();
  true_helK->Write();
  rec_helK->Write();
  gen_helR->Write();
  true_helR->Write();
  rec_helR->Write();
}




void test2_1210032007(){

  TString inputfile("../1210032007.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("B2KstRho");
  TTree* mctree = (TTree*)sample->Get("MCB2KstRho");

  TFile* outputFile = new TFile("1210032007_Validation.root","RECREATE");

  plot(tree, mctree, outputFile);


  outputFile->Close();

}
