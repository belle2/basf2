/*
<header>
<input>../ana-dstars.root, ../ana-xi-lambda.root, ../ana-jpsiks.root</input>
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

void plotROC(TFile* pfile, TTree* ptree, TFile* pikfile, TTree* piktree, TFile* eemmfile, TTree* mutree, TTree* etree){

  // for future compatibility if we split leptons
  TFile* eeeefile = eemmfile;

  // where to find particle samples in the TTrees
  TString pibranch("DST_D0_pi");
  TString kbranch("DST_D0_K");
  TString pbranch("Xi_Lambda0_p");

  TString mubranch("B0_Jpsi_mu0");
  TString ebranch("B0_Jpsi_e0");
  TString leptpibranch("B0_K_S0_pi0");

  TString PIDp("PIDp");
  TString PIDpi("PIDpi");
  TString PIDk("PIDk");
  TString PIDmu("PIDmu");
  TString PIDe("PIDe");
  TString pidcut("0.5");

  // define some cuts
  TString selectionCuts("(Xi_Lambda0_Rho>0.5)");
  TString xiCuts("abs(Xi_M-1.3216)<0.005");

  TString pikcuts("abs(DST_D0_M-1.86484)<0.012&&abs(DST_M-DST_D0_M-0.14543)<0.00075");
  TString pcuts(selectionCuts+"&&"+xiCuts);
  TString jpsicuts("abs(B0_Jpsi_M-3.09692)<0.093&&abs(B0_K_S0_M-0.49761)<0.024"); //rough cuts 
  TString mucuts("abs("+mubranch+"_mcPDG)==13 && "+jpsicuts);
  TString ecuts("abs("+ebranch+"_mcPDG)==11 && "+jpsicuts);
  TString lepttruthpi("abs("+leptpibranch+"_mcPDG)==211 && "+jpsicuts);

  const int pdiv = 6; // number of momentum ranges to test
  TGraph pcutgr[pdiv], picutgr[pdiv], kcutgr[pdiv], 
         mucutgr[pdiv], ecutgr[pdiv];
  TGraph ppigr[pdiv], pkgr[pdiv], pikgr[pdiv], kpigr[pdiv], 
         mupigr[pdiv], epigr[pdiv];

  const int piddiv = 20; // number of PID cuts to test (divisions in ROC plots)
  double cutvalue[piddiv];
  double binwidth = 1.0/piddiv;
  double pieff[piddiv], pipfake[piddiv], pikfake[piddiv];
  double keff[piddiv], kpfake[piddiv], kpifake[piddiv];
  double peff[piddiv], ppifake[piddiv];
  double mueff[piddiv], pimufake[piddiv];
  double eeff[piddiv], piefake[piddiv];


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

      // muons
      eemmfile->cd();
      if( j == 5 ) range = TString::Format("100,0.8,4.0");
      mutree->Project("hmuall("+range+")",mubranch+"_P",mucuts);
      mutree->Project("hmuacc("+range+")",mubranch+"_P",mubranch+"_"+PIDmu+">"+pidcut+"&&"+mucuts);
      //select from jpsi sample particles that pass the muon ID but are actually pions (I use the Ks side of the decay for this)
      mutree->Project("hpimufake("+range+")",leptpibranch+"_P",leptpibranch+"_"+PIDmu+">"+pidcut+"&&"+lepttruthpi);      
      TH1F* hmuall = (TH1F*)eemmfile->Get("hmuall");
      TH1F* hmuacc = (TH1F*)eemmfile->Get("hmuacc");
      TH1F* hpimufake = (TH1F*)eemmfile->Get("hpimufake");
      mueff[i] = hmuacc->Integral()/hmuall->Integral();
      pimufake[i] = hpimufake->Integral()/hmuall->Integral();
      // electrons
      if( j == 5 ) range = TString::Format("100,0.5,4.0");
      etree->Project("heall("+range+")",ebranch+"_P",ecuts);
      etree->Project("heacc("+range+")",ebranch+"_P",ebranch+"_"+PIDe+">"+pidcut+"&&"+ecuts);
      //select from jpsi sample particles that pass the electron ID but are actually pions (I use the Ks side of the decay for this)
      etree->Project("hpiefake("+range+")",leptpibranch+"_P",leptpibranch+"_"+PIDe+">"+pidcut+"&&"+lepttruthpi);      
      TH1F* heall = (TH1F*)eeeefile->Get("heall");
      TH1F* heacc = (TH1F*)eeeefile->Get("heacc");
      TH1F* hpiefake = (TH1F*)eeeefile->Get("hpiefake");
      eeff[i] = heacc->Integral()/heall->Integral();
      piefake[i] = hpiefake->Integral()/heall->Integral();
    }

    // PID cut vs efficiency to determine benchmarks
    pcutgr[j] = TGraph(piddiv,peff,cutvalue);
    picutgr[j] = TGraph(piddiv,pieff,cutvalue);
    kcutgr[j] = TGraph(piddiv,keff,cutvalue);
    mucutgr[j] = TGraph(piddiv,mueff,cutvalue);
    ecutgr[j] = TGraph(piddiv,eeff,cutvalue);

    // ROC plots to determine fake rates
    ppigr[j] = TGraph(piddiv,peff,pipfake);
    pkgr[j]  = TGraph(piddiv,peff,kpfake);
    pikgr[j] = TGraph(piddiv,pieff,kpifake);
    kpigr[j] = TGraph(piddiv,keff,pikfake);
    mupigr[j] = TGraph(piddiv,mueff,pimufake);
    epigr[j] = TGraph(piddiv,eeff,piefake);
  }

  TFile* outputFile = new TFile("standardParticlesValidation.root","RECREATE");
  outputFile->cd();

  const char* xlabel[] = {"85%","90%","95%","99%"};
  const double xbinval[] = {0.85,0.90,0.95,0.99};

  TH1F* hpcut  = new TH1F("hpcut","PID_p cut efficiency",4,0,4);
  TH1F* hpicut = new TH1F("hpicut","PID_pi cut efficiency",4,0,4);
  TH1F* hkcut  = new TH1F("hkcut","PID_k cut efficiency",4,0,4);
  TH1F* hmucut  = new TH1F("hmucut","PID_mu cut efficiency",4,0,4);
  TH1F* hecut  = new TH1F("hecut","PID_e cut efficiency",4,0,4);

  TH1F* hppifake  = new TH1F("hppifake","p/#pi fake rate",4,0,4);
  TH1F* hpkfake  = new TH1F("hpkfake","p/K fake rate",4,0,4);
  TH1F* hpikfake = new TH1F("hpikfake","#pi/K fake rate",4,0,4);
  TH1F* hkpifake = new TH1F("hkpifake","K/#pi fake rate",4,0,4);
  TH1F* hmupifake  = new TH1F("hmupifake","#mu/#pi fake rate",4,0,4);
  TH1F* hepifake  = new TH1F("hepifake","e/#pi fake rate",4,0,4);

  for( int i = 0; i < 4; ++i ){
    hpcut->SetBinContent(i+1,pcutgr[5].Eval(xbinval[i]));
    hpicut->SetBinContent(i+1,picutgr[5].Eval(xbinval[i]));
    hkcut->SetBinContent(i+1,kcutgr[5].Eval(xbinval[i]));
    hmucut->SetBinContent(i+1,mucutgr[5].Eval(xbinval[i]));
    hecut->SetBinContent(i+1,ecutgr[5].Eval(xbinval[i]));

    hppifake->SetBinContent(i+1,ppigr[5].Eval(xbinval[i]));
    hpkfake->SetBinContent(i+1,pkgr[5].Eval(xbinval[i]));
    hpikfake->SetBinContent(i+1,pikgr[5].Eval(xbinval[i]));
    hkpifake->SetBinContent(i+1,kpigr[5].Eval(xbinval[i]));
    hmupifake->SetBinContent(i+1,mupigr[5].Eval(xbinval[i]));
    hepifake->SetBinContent(i+1,epigr[5].Eval(xbinval[i]));

    hpcut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpicut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hkcut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hmucut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hecut->GetXaxis()->SetBinLabel(i+1,xlabel[i]);

    hppifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpkfake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hpikfake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hkpifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hmupifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
    hepifake->GetXaxis()->SetBinLabel(i+1,xlabel[i]);
  }

  hpcut->SetXTitle("Proton efficiency");
  hpicut->SetXTitle("Pion efficiency");
  hkcut->SetXTitle("Kaon efficiency");
  hmucut->SetXTitle("Muon efficiency");
  hecut->SetXTitle("Electron efficiency");

  hpcut->SetYTitle("PIDp cut value");
  hpicut->SetYTitle("PIDpi cut value");
  hkcut->SetYTitle("PIDk cut value");
  hmucut->SetYTitle("PIDmu cut value");
  hecut->SetYTitle("PIDe cut value");

  hppifake->SetXTitle("Proton efficiency");
  hpkfake->SetXTitle("Proton efficiency");
  hpikfake->SetXTitle("Pion efficiency");
  hkpifake->SetXTitle("Kaon efficiency");
  hmupifake->SetXTitle("Muon efficiency");
  hepifake->SetXTitle("Electron efficiency");

  hppifake->SetYTitle("Pion fake rate");
  hpkfake->SetYTitle("Kaon fake rate");
  hpikfake->SetYTitle("Kaon fake rate");
  hkpifake->SetYTitle("Pion fake rate");
  hmupifake->SetYTitle("Pion fake rate");
  hepifake->SetYTitle("Pion fake rate");

  hpcut->GetListOfFunctions()->Add(new TNamed("Description", hpcut->GetTitle()));
  hpicut->GetListOfFunctions()->Add(new TNamed("Description", hpicut->GetTitle()));
  hkcut->GetListOfFunctions()->Add(new TNamed("Description", hkcut->GetTitle()));
  hmucut->GetListOfFunctions()->Add(new TNamed("Description", hmucut->GetTitle()));
  hecut->GetListOfFunctions()->Add(new TNamed("Description", hecut->GetTitle()));

  hppifake->GetListOfFunctions()->Add(new TNamed("Description", hppifake->GetTitle()));
  hpkfake->GetListOfFunctions()->Add(new TNamed("Description", hpkfake->GetTitle()));
  hpikfake->GetListOfFunctions()->Add(new TNamed("Description", hpikfake->GetTitle()));
  hkpifake->GetListOfFunctions()->Add(new TNamed("Description", hkpifake->GetTitle()));
  hmupifake->GetListOfFunctions()->Add(new TNamed("Description", hmupifake->GetTitle()));
  hepifake->GetListOfFunctions()->Add(new TNamed("Description", hepifake->GetTitle()));

  hpcut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpicut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hkcut->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hmucut->GetListOfFunctions()->Add(new TNamed("Contact","francesco.tenchini@unimelb.edu.au"));
  hecut->GetListOfFunctions()->Add(new TNamed("Contact","francesco.tenchini@unimelb.edu.au"));
  //add contacts for leptons

  hppifake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpkfake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hpikfake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hkpifake->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hmupifake->GetListOfFunctions()->Add(new TNamed("Contact","francesco.tenchini@unimelb.edu.au"));
  hepifake->GetListOfFunctions()->Add(new TNamed("Contact","francesco.tenchini@unimelb.edu.au"));
  //add contact for leptons

  hpcut->Write();
  hpicut->Write();
  hkcut->Write();
  hmucut->Write();
  hecut->Write();

  hppifake->Write();
  hpkfake->Write();
  hpikfake->Write();
  hkpifake->Write();
  hmupifake->Write();
  hepifake->Write();

  outputFile->Close();

  TCanvas* c = new TCanvas("leptons","leptons",800,600) ;
  c->Divide(2,2);
  TMultiGraph *mucutmg = new TMultiGraph();
  mucutmg->SetTitle("mucutmg");
  TMultiGraph *ecutmg = new TMultiGraph();
  ecutmg->SetTitle("ecutmg");
  TMultiGraph *mupimg = new TMultiGraph();
  mupimg->SetTitle("mupimg");
  TMultiGraph *epimg = new TMultiGraph();  
  epimg->SetTitle("epimg");
  for( int i = 0; i <= 5; ++i ){
    //    c->cd(1);
    mucutgr[i].SetMarkerStyle(20);
    mucutgr[i].SetMarkerColor(i+2);
    mucutmg->Add(&mucutgr[i]);
    //    c->cd(2);
    ecutgr[i].SetMarkerStyle(20);
    ecutgr[i].SetMarkerColor(i+2);
    ecutmg->Add(&ecutgr[i]);
    //    c->cd(3);
    mupigr[i].SetMarkerStyle(20);
    mupigr[i].SetMarkerColor(i+2);
    mupimg->Add(&mupigr[i]);
    //    c->cd(4);
    epigr[i].SetMarkerStyle(20);
    epigr[i].SetMarkerColor(i+2);
    epimg->Add(&epigr[i]);
  }
  c->cd(1);
  mucutmg->Draw("APL");
  c->cd(2);
  ecutmg->Draw("APL");
  c->cd(3);
  mupimg->Draw("APL");
  mupimg->SetMaximum(0.8);
  c->cd(4);
  epimg->Draw("APL");
  epimg->SetMaximum(0.35);
  c->SaveAs("lepton_roc.pdf");
}

void test2_Validation_StandardParticles_withLeptons(){

  // pion, kaon, and proton samples
  TString pikfile("../ana-dstars.root");
  TString pfile("../ana-xi-lambda.root");
  TString eemmfile("../ana-jpsiks.root");

  TFile* pikSample = new TFile(pikfile);
  TTree* piktree = (TTree*)pikSample->Get("dsttree");
  TFile* pSample = new TFile(pfile);
  TTree* ptree = (TTree*)pSample->Get("xitree");

  TFile* eemmSample = new TFile(eemmfile);
  TTree* mutree = (TTree*)eemmSample->Get("jpsiks_mu");
  TTree* etree = (TTree*)eemmSample->Get("jpsiks_e");

  plotROC(pSample, ptree, pikSample, piktree, eemmSample, mutree, etree);
}
