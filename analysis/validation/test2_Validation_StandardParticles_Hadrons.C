/*
<header>
<input>../ana-dstars.root, ../ana-xi-lambda.root</input>
<output>standardParticlesValidation.root</output>
<contact>Jake Bennett, jvbennett@cmu.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Validation_StandardParticles.C
// Check the PID cut values for given efficiencies and fakes
//
// Constributor: Jake Bennett
// October 3, 2016
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plotROC(TFile* pfile, TTree* ptree, TFile* pikfile, TTree* piktree){
 
  // where to find particle samples in the TTrees
  TString pibranch("DST_D0_pi");
  TString kbranch("DST_D0_K");
  TString pbranch("Xi_Lambda0_p");

  TString PIDp("PIDp");
  TString PIDpi("PIDpi");
  TString PIDk("PIDk");
  TString pidcut("0.5");

  // define some cuts
  TString selectionCuts("(Xi_Lambda0_Rho>0.5)");
  TString xiCuts("abs(Xi_M-1.3216)<0.005");

  TString pikcuts("abs(DST_D0_M-1.86484)<0.012&&abs(DST_M-DST_D0_M-0.14543)<0.00075");
  TString pcuts(selectionCuts+"&&"+xiCuts);

  const int pdiv = 6; // number of momentum ranges to test
  TGraph pcutgr[pdiv], picutgr[pdiv], kcutgr[pdiv];
  TGraph ppigr[pdiv], pkgr[pdiv], pikgr[pdiv], kpigr[pdiv];

  const int piddiv = 20; // number of PID cuts to test (divisions in ROC plots)
  double cutvalue[piddiv];
  double binwidth = 1.0/piddiv;
  double pieff[piddiv], pipfake[piddiv], pikfake[piddiv];
  double keff[piddiv], kpfake[piddiv], kpifake[piddiv];
  double peff[piddiv], ppifake[piddiv];

  // ---------- Momentum distributions (for efficiency determination) ----------
  for( int j = 0; j <= 5; ++j ){
    TString range = TString::Format("100,0.05,%f",j*1.0);
    if( j == 0 ) range = TString::Format("100,1.0,2.0");
    else if( j == 5 ) range = TString::Format("100,1.0,4.0");

    // check different PID cuts
    for( int i = 0; i < piddiv; ++i ){
      pidcut = TString::Format("%f",binwidth*i);
      if( j == 0 )
	cutvalue[i] = binwidth*i;

      // ---------- Momentum distributions (for efficiency determination) ----------
      // protons
      pfile->cd();
      ptree->Project("hpall("+range+")",pbranch+"_P",pcuts);
      ptree->Project("hpacc("+range+")",pbranch+"_P",pbranch+"_"+PIDp+">"+pidcut+"&&"+pcuts);
      ptree->Project("hppifake("+range+")",pbranch+"_P",pbranch+"_"+PIDpi+">"+pidcut+"&&"+pcuts);
      TH1F* hpall = (TH1F*)pfile->Get("hpall");
      TH1F* hpacc = (TH1F*)pfile->Get("hpacc");
      TH1F* hppifake = (TH1F*)pfile->Get("hppifake");
      peff[i] = hpacc->Integral()/hpall->Integral();
      ppifake[i] = hppifake->Integral()/hpall->Integral();

      // pions
      pikfile->cd();
      piktree->Project("hpiall("+range+")",pibranch+"_P",pikcuts);
      piktree->Project("hpiacc("+range+")",pibranch+"_P",pibranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpipfake("+range+")",pibranch+"_P",pibranch+"_"+PIDp+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpikfake("+range+")",pibranch+"_P",pibranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
      TH1F* hpiall = (TH1F*)pikfile->Get("hpiall");
      TH1F* hpiacc = (TH1F*)pikfile->Get("hpiacc");
      TH1F* hpipfake = (TH1F*)pikfile->Get("hpipfake");
      TH1F* hpikfake = (TH1F*)pikfile->Get("hpikfake");
      pieff[i] = hpiacc->Integral()/hpiall->Integral();
      pipfake[i] = hpipfake->Integral()/hpiall->Integral();
      pikfake[i] = hpikfake->Integral()/hpiall->Integral();

      // kaons
      piktree->Project("hkall("+range+")",kbranch+"_P",pikcuts);
      piktree->Project("hkacc("+range+")",kbranch+"_P",kbranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hkpfake("+range+")",kbranch+"_P",kbranch+"_"+PIDp+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hkpifake("+range+")",kbranch+"_P",kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
      TH1F* hkall = (TH1F*)pikfile->Get("hkall");
      TH1F* hkacc = (TH1F*)pikfile->Get("hkacc");
      TH1F* hkpfake = (TH1F*)pikfile->Get("hkpfake");
      TH1F* hkpifake = (TH1F*)pikfile->Get("hkpifake");
      keff[i] = hkacc->Integral()/hkall->Integral();
      kpfake[i] = hkpfake->Integral()/hkall->Integral();
      kpifake[i] = hkpifake->Integral()/hkall->Integral();
    }

    // PID cut vs efficiency to determine benchmarks
    pcutgr[j] = TGraph(piddiv,peff,cutvalue);
    picutgr[j] = TGraph(piddiv,pieff,cutvalue);
    kcutgr[j] = TGraph(piddiv,keff,cutvalue);

    // ROC plots to determine fake rates
    ppigr[j] = TGraph(piddiv,peff,pipfake);
    pkgr[j]  = TGraph(piddiv,peff,kpfake);
    pikgr[j] = TGraph(piddiv,pieff,kpifake);
    kpigr[j] = TGraph(piddiv,keff,pikfake);
  }

  TFile* outputFile = new TFile("standardParticlesValidation.root","RECREATE");
  outputFile->cd();

  const char* xlabel[] = {"85%","90%","95%","99%"};
  const double xbinval[] = {0.85,0.90,0.95,0.99};

  TH1F* hpcut  = new TH1F("hpcut","PID_p cut efficiency",4,0,4);
  TH1F* hpicut = new TH1F("hpicut","PID_pi cut efficiency",4,0,4);
  TH1F* hkcut  = new TH1F("hkcut","PID_k cut efficiency",4,0,4);

  TH1F* hppifake  = new TH1F("hppifake","p/#pi fake rate",4,0,4);
  TH1F* hpkfake  = new TH1F("hpkfake","p/K fake rate",4,0,4);
  TH1F* hpikfake = new TH1F("hpikfake","#pi/K fake rate",4,0,4);
  TH1F* hkpifake  = new TH1F("hkpifake","K/#pi fake rate",4,0,4);

  for( int i = 0; i < 4; ++i ){
    hpcut->SetBinContent(i+1,pcutgr[5].Eval(xbinval[i]));
    hpicut->SetBinContent(i+1,picutgr[5].Eval(xbinval[i]));
    hkcut->SetBinContent(i+1,kcutgr[5].Eval(xbinval[i]));

    hppifake->SetBinContent(i+1,ppigr[5].Eval(xbinval[i]));
    hpkfake->SetBinContent(i+1,pkgr[5].Eval(xbinval[i]));
    hpikfake->SetBinContent(i+1,pikgr[5].Eval(xbinval[i]));
    hkpifake->SetBinContent(i+1,kpigr[5].Eval(xbinval[i]));

    hpcut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpicut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hkcut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);

    hppifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpkfake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpikfake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hkpifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
  }

  hpcut->SetXTitle("Proton efficiency");
  hpicut->SetXTitle("Pion efficiency");
  hkcut->SetXTitle("Kaon efficiency");

  hpcut->SetYTitle("PIDp cut value");
  hpicut->SetYTitle("PIDpi cut value");
  hkcut->SetYTitle("PIDk cut value");

  hppifake->SetXTitle("Proton efficiency");
  hpkfake->SetXTitle("Proton efficiency");
  hpikfake->SetXTitle("Pion efficiency");
  hkpifake->SetXTitle("Kaon efficiency");

  hppifake->SetYTitle("Pion fake rate");
  hpkfake->SetYTitle("Kaon fake rate");
  hpikfake->SetYTitle("Kaon fake rate");
  hkpifake->SetYTitle("Pion fake rate");

  hpcut->GetListOfFunctions()->Add(new TNamed("Description", hpcut->GetTitle()));
  hpicut->GetListOfFunctions()->Add(new TNamed("Description", hpicut->GetTitle()));
  hkcut->GetListOfFunctions()->Add(new TNamed("Description", hkcut->GetTitle()));

  hppifake->GetListOfFunctions()->Add(new TNamed("Description", hppifake->GetTitle()));
  hpkfake->GetListOfFunctions()->Add(new TNamed("Description", hpkfake->GetTitle()));
  hpikfake->GetListOfFunctions()->Add(new TNamed("Description", hpikfake->GetTitle()));
  hkpifake->GetListOfFunctions()->Add(new TNamed("Description", hkpifake->GetTitle()));

  hpcut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpicut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hkcut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));

  hppifake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpkfake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpikfake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hkpifake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));

  hpcut->Write();
  hpicut->Write();
  hkcut->Write();

  hppifake->Write();
  hpkfake->Write();
  hpikfake->Write();
  hkpifake->Write();

  outputFile->Close();
}

void test2_Validation_StandardParticles(){

  // pion, kaon, and proton samples
  TString pikfile("../ana-dstars.root");
  TString pfile("../ana-xi-lambda.root");

  TFile* pikSample = new TFile(pikfile);
  TTree* piktree = (TTree*)pikSample->Get("dsttree");
  TFile* pSample = new TFile(pfile);
  TTree* ptree = (TTree*)pSample->Get("xitree");

  plotROC(pSample, ptree, pikSample, piktree);
}
