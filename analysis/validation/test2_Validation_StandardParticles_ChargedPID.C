/*
<header>
<input>../ana-dstars.root, ../ana-xi-lambda.root</input>
<output>standardParticlesValidation_ChargedPID.root</output>
<contact>Jan Strube, jan.strube@desy.de</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Check the PID cut values for given efficiencies and fakes
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
  TString PIDe("PIDe");
  TString PIDmu("PIDmu");
  TString pidcut("0.5");

  // define some cuts
  TString selectionCuts("(Xi_Lambda0_Rho>0.5)");
  TString xiCuts("abs(Xi_M-1.3216)<0.005");
  TString pcuts(selectionCuts+"&&"+xiCuts);

  TString pikcuts("abs(DST_D0_M-1.86484)<0.012&&abs(DST_M-DST_D0_M-0.14543)<0.00075");

  const int pdiv = 6; // number of momentum ranges to test
  TGraph* pcutgr[pdiv];
  TGraph* picutgr[pdiv];
  TGraph* kcutgr[pdiv];
  TGraph* ppigr[pdiv];
  TGraph* pkgr[pdiv];
  TGraph* pikgr[pdiv];
  TGraph* pimugr[pdiv];
  TGraph* piegr[pdiv];
  TGraph* kpigr[pdiv];
  TGraph* kpgr[pdiv];

  const int piddiv = 20; // number of PID cuts to test (divisions in ROC plots)
  double cutvalue[piddiv];
  double binwidth = 1.0/piddiv;
  double pieff[piddiv];
  double keff[piddiv];
  double peff[piddiv]; 
  //double pipfake[piddiv];
  double pikfake[piddiv];
  double pimufake[piddiv];
  double piefake[piddiv];
  double kpfake[piddiv];
  double kpifake[piddiv];
  double ppifake[piddiv];
  double pkfake[piddiv];


  // ---------- Momentum distributions (for efficiency determination) ----------
  for ( int j = 0; j < pdiv; ++j ){
    TString range = TString::Format("100,0.05,%f",j*1.0);
    if ( j == 0 ) range = TString::Format("100,1.0,2.0");
    else if ( j == pdiv - 1 ) range = TString::Format("100,1.0,4.0");

    // check different PID cuts
    for ( int i = 0; i < piddiv; ++i ){
      pidcut = TString::Format("%f",binwidth*i);
      if ( j == 0 ) cutvalue[i] = binwidth*i;

      // ---------- Momentum distributions (for efficiency determination) ----------
      // protons
      pfile->cd();
      ptree->Project("hpall("+range+")",pbranch+"_P",pcuts);
      ptree->Project("hpacc("+range+")",pbranch+"_P",pbranch+"_"+PIDp+">"+pidcut+"&&"+pcuts);
      ptree->Project("hppifake("+range+")",pbranch+"_P",pbranch+"_"+PIDpi+">"+pidcut+"&&"+pcuts);
      ptree->Project("hpkfake("+range+")",pbranch+"_P",pbranch+"_"+PIDk+">"+pidcut+"&&"+pcuts);
      TH1F* hpall = (TH1F*)pfile->Get("hpall");
      TH1F* hpacc = (TH1F*)pfile->Get("hpacc");
      TH1F* hppifake = (TH1F*)pfile->Get("hppifake");
      TH1F* hpkfake = (TH1F*)pfile->Get("hpkfake");
      peff[i] = hpacc->Integral()/hpall->Integral();
      ppifake[i] = hppifake->Integral()/hpall->Integral();
      pkfake[i] = hpkfake->Integral()/hpall->Integral();

      // pions
      pikfile->cd();
      piktree->Project("hpiall("+range+")",pibranch+"_P",pikcuts);
      piktree->Project("hpiacc("+range+")",pibranch+"_P",pibranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpipfake("+range+")",pibranch+"_P",pibranch+"_"+PIDp+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpikfake("+range+")",pibranch+"_P",pibranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpimufake("+range+")",pibranch+"_P",pibranch+"_"+PIDmu+">"+pidcut+"&&"+pikcuts);
      piktree->Project("hpiefake("+range+")",pibranch+"_P",pibranch+"_"+PIDe+">"+pidcut+"&&"+pikcuts);
      TH1F* hpiall = (TH1F*)pikfile->Get("hpiall");
      TH1F* hpiacc = (TH1F*)pikfile->Get("hpiacc");
      //TH1F* hpipfake = (TH1F*)pikfile->Get("hpipfake");
      TH1F* hpikfake = (TH1F*)pikfile->Get("hpikfake");
      TH1F* hpimufake = (TH1F*)pikfile->Get("hpimufake");
      TH1F* hpiefake = (TH1F*)pikfile->Get("hpiefake");

      pieff[i] = hpiacc->Integral()/hpiall->Integral();
      //pipfake[i] = hpipfake->Integral()/hpiall->Integral();
      pikfake[i] = hpikfake->Integral()/hpiall->Integral();
      pimufake[i] = hpimufake->Integral()/hpiall->Integral();
      piefake[i] = hpiefake->Integral()/hpiall->Integral();

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
    pcutgr[j] = new TGraph(piddiv,peff,cutvalue);
    picutgr[j] = new TGraph(piddiv,pieff,cutvalue);
    kcutgr[j] = new TGraph(piddiv,keff,cutvalue);

    // ROC plots to determine fake rates
    ppigr[j] = new TGraph(piddiv,peff,ppifake);
    pkgr[j]  = new TGraph(piddiv,peff,pkfake);
    pikgr[j] = new TGraph(piddiv,pieff,pikfake);
    kpigr[j] = new TGraph(piddiv,keff,kpifake);
    kpgr[j] = new TGraph(piddiv,keff,kpfake);
    pimugr[j] = new TGraph(piddiv,keff,pimufake);
    piegr[j] = new TGraph(piddiv,keff,piefake);

  }

  TFile* outputFile = new TFile("standardParticlesValidation_ChargedPID.root","RECREATE");
  outputFile->cd();
  TH1F* Kall_vs_momentum = new TH1F("Kall_vs_momentum", "K rate vs. momentum;p_{K} (GeV);#tracks", 100, 0.5, 4.0);
  TH1F* piall_vs_momentum = new TH1F("piall_vs_momentum", "#pi rate vs. momentum;p_{#pi} (GeV);#tracks", 100, 0.5, 4.0);
  piktree->Project("piall_vs_momentum", pibranch+"_P", pikcuts);
  piktree->Project("Kall_vs_momentum", kbranch+"_P", pikcuts);
  outputFile->WriteTObject(piall_vs_momentum);
  outputFile->WriteTObject(Kall_vs_momentum);

  TH1F* Kpifake_vs_momentum = new TH1F("Kpifake_vs_momentum", "K/#pi fake rate vs. momentum;p_{K} (GeV);#pi ID efficiency", 100, 0.5, 4.0);
  TH1F* Kpfake_vs_momentum = new TH1F("Kpfake_vs_momentum", "K/p fake rate vs. momentum;p_{K} (GeV);p ID efficiency", 100, 0.5, 4.0);
  TH1F* piKfake_vs_momentum = new TH1F("piKfake_vs_momentum", "#pi/K fake rate vs. momentum;p_{#pi} (GeV);K ID efficiency", 100, 0.5, 4.0);
  TH1F* pimufake_vs_momentum = new TH1F("pimufake_vs_momentum", "#pi/#mu fake rate vs. momentum;p_{#pi} (GeV);#mu ID efficiency", 100, 0.5, 4.0);
  TH1F* piefake_vs_momentum = new TH1F("piefake_vs_momentum", "#pi/e fake rate vs. momentum;p_{#pi} (GeV);e ID efficiency", 100, 0.5, 4.0);

  piktree->Project("piKfake_vs_momentum", pibranch+"_P", pibranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
  piktree->Project("piefake_vs_momentum", pibranch+"_P", pibranch+"_"+PIDmu+">"+pidcut+"&&"+pikcuts);
  piktree->Project("pimufake_vs_momentum", pibranch+"_P", pibranch+"_"+PIDe+">"+pidcut+"&&"+pikcuts);
  piktree->Project("Kpifake_vs_momentum", kbranch+"_P", kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
  piktree->Project("Kpfake_vs_momentum", kbranch+"_P", kbranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
  Kpifake_vs_momentum->Divide(Kall_vs_momentum);
  Kpfake_vs_momentum->Divide(Kall_vs_momentum);
  piKfake_vs_momentum->Divide(piall_vs_momentum); 
  pimufake_vs_momentum->Divide(piall_vs_momentum); 
  piefake_vs_momentum->Divide(piall_vs_momentum); 
  Kpfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  Kpifake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  piKfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  piefake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  pimufake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  Kpfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Description", "Rate of Kaons faking protons vs. momentum"));
  Kpifake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Description", "Rate of Kaons faking pions vs. momentum"));
  piKfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking Kaons vs. momentum"));
  piefake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking electrons vs. momentum"));
  pimufake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking muons vs. momentum"));
  Kpfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  Kpifake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  piKfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  piefake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  pimufake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  Kpfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  Kpifake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  piKfake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  piefake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  pimufake_vs_momentum->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  outputFile->WriteTObject(piKfake_vs_momentum);
  outputFile->WriteTObject(piefake_vs_momentum);
  outputFile->WriteTObject(pimufake_vs_momentum);
  outputFile->WriteTObject(Kpfake_vs_momentum);
  outputFile->WriteTObject(Kpifake_vs_momentum);

  TH1F* kplusRecoEff_vs_cosTheta = new TH1F("kplusRecoEff_vs_cosTheta", "charge-dependent Kaon reconstruction efficiency;cos(#theta_{K});#epsilon(K+)-#epsilon(K-)", 200, -1, 1);
  TH1F* kminusRecoEff_vs_cosTheta = new TH1F("kminusRecoEff_vs_cosTheta", "charge-dependent Kaon reconstruction efficiency", 200, -1, 1);
  TH1F* krecoEff_vs_cosTheta = new TH1F("krecoEff_vs_cosTheta", "charge-dependent Kaon reconstruction efficiency", 200, -1, 1);
  piktree->Project("kplusRecoEff_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", "("+kbranch+"_charge > 0) && "+kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
  piktree->Project("kminusRecoEff_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", "("+kbranch+"_charge < 0) && "+kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
  piktree->Project("krecoEff_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
  kplusRecoEff_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  kplusRecoEff_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Difference in reco efficiency between K+ and K-"));
  kplusRecoEff_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  kplusRecoEff_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "flat line across the whole range"));
  kplusRecoEff_vs_cosTheta->Divide(krecoEff_vs_cosTheta);
  kminusRecoEff_vs_cosTheta->Divide(krecoEff_vs_cosTheta);
  kplusRecoEff_vs_cosTheta->Add(kminusRecoEff_vs_cosTheta, -1);
  outputFile->WriteTObject(kplusRecoEff_vs_cosTheta);

  TH1F* Kall_vs_cosTheta = new TH1F("Kall_vs_cosTheta", "K rate vs. cosTheta;cos(#theta_{K});#tracks", 200, -1, 1);
  TH1F* piall_vs_cosTheta = new TH1F("piall_vs_cosTheta", "#pi rate vs. cosTheta;cos(#theta_{#pi});#tracks", 200, -1, 1);
  piktree->Project("Kall_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", pikcuts);
  piktree->Project("piall_vs_cosTheta", pibranch+"_P4[2]/"+pibranch+"_P", pikcuts);
  outputFile->WriteTObject(Kall_vs_cosTheta);
  outputFile->WriteTObject(piall_vs_cosTheta);

  TH1F* Kpifake_vs_cosTheta = new TH1F("Kpifake_vs_cosTheta", "K/#pi fake rate vs. cosTheta;cos(#theta_{K});#pi ID efficiency", 200, -1, 1);
  TH1F* Kpfake_vs_cosTheta = new TH1F("Kpfake_vs_cosTheta", "K/p fake rate vs. cosTheta;cos(#theta_{K});p ID efficiency", 200, -1, 1);
  TH1F* piKfake_vs_cosTheta = new TH1F("piKfake_vs_cosTheta", "#pi/K fake rate vs. cosTheta;cos(#theta_{#pi});K ID efficiency", 200, -1, 1);
  TH1F* pimufake_vs_cosTheta = new TH1F("pimufake_vs_cosTheta", "#pi/#mu fake rate vs. cosTheta;cos(#theta_{#pi});#mu ID efficiency", 200, -1, 1);
  TH1F* piefake_vs_cosTheta = new TH1F("piefake_vs_cosTheta", "#pi/e fake rate vs. cosTheta;cos(#theta_{#pi});e ID efficiency", 200, -1, 1);
  piktree->Project("piKfake_vs_cosTheta", pibranch+"_P4[2]/"+pibranch+"_P", pibranch+"_"+PIDk+">"+pidcut+"&&"+pikcuts);
  piktree->Project("piefake_vs_cosTheta", pibranch+"_P4[2]/"+pibranch+"_P", pibranch+"_"+PIDmu+">"+pidcut+"&&"+pikcuts);
  piktree->Project("pimufake_vs_cosTheta", pibranch+"_P4[2]/"+pibranch+"_P", pibranch+"_"+PIDe+">"+pidcut+"&&"+pikcuts);
  piktree->Project("Kpifake_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", kbranch+"_"+PIDpi+">"+pidcut+"&&"+pikcuts);
  piktree->Project("Kpfake_vs_cosTheta", kbranch+"_P4[2]/"+kbranch+"_P", kbranch+"_"+PIDp+">"+pidcut+"&&"+pikcuts);
  Kpfake_vs_cosTheta->Divide(Kall_vs_cosTheta);
  Kpifake_vs_cosTheta->Divide(Kall_vs_cosTheta);
  piKfake_vs_cosTheta->Divide(piall_vs_cosTheta);
  piefake_vs_cosTheta->Divide(piall_vs_cosTheta);
  pimufake_vs_cosTheta->Divide(piall_vs_cosTheta);
  Kpfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  Kpifake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  piKfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  piefake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  pimufake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Meta", "nostats, expert"));
  Kpfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  Kpifake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  piKfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  piefake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  pimufake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Check", "Currently low stats, but should be consistent across versions"));
  Kpfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Rate of Kaons faking protons vs. cosTheta"));
  Kpifake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Rate of Kaons faking pions vs. cosTheta"));
  piKfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking Kaons vs. cosTheta"));
  piefake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking electrons vs. cosTheta"));
  pimufake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Description", "Rate of pions faking muons vs. cosTheta"));
  Kpfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  Kpifake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  piKfake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  piefake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  pimufake_vs_cosTheta->GetListOfFunctions()->Add(new TNamed("Contact", "jan.strube@desy.de"));
  outputFile->WriteTObject(piKfake_vs_cosTheta);
  outputFile->WriteTObject(piefake_vs_cosTheta);
  outputFile->WriteTObject(pimufake_vs_cosTheta);
  outputFile->WriteTObject(Kpfake_vs_cosTheta);
  outputFile->WriteTObject(Kpifake_vs_cosTheta);


  // The following section goes over the different efficiency bins
  const vector<string> xlabel = {"85%","90%","95%","99%"};
  const vector<double> xbinval = {0.85,0.90,0.95,0.99};
  const size_t nBenchmarks = xbinval.size();

  TH1F* hpcut  = new TH1F("hpcut","PID_p cut efficiency",nBenchmarks,0,nBenchmarks);
  TH1F* hpicut = new TH1F("hpicut","PID_pi cut efficiency",nBenchmarks,0,nBenchmarks);
  TH1F* hkcut  = new TH1F("hkcut","PID_k cut efficiency",nBenchmarks,0,nBenchmarks);

  // fake rates at different efficiency benchmarks
  TH1F* hkpfake_eff  = new TH1F("hkpfake_eff","K/p fake rate",nBenchmarks,0,nBenchmarks);
  TH1F* hkpifake_eff  = new TH1F("hkpifake_eff","K/#pi fake rate",nBenchmarks,0,nBenchmarks);
  TH1F* hpikfake_eff = new TH1F("hpikfake_eff","#pi/K fake rate",nBenchmarks,0,nBenchmarks);
  TH1F* hpimufake_eff  = new TH1F("hpimufake_eff","#pi/#mu fake rate",nBenchmarks,0,nBenchmarks);
  TH1F* hpiefake_eff  = new TH1F("hpiefake_eff","#pi/e fake rate",nBenchmarks,0,nBenchmarks);

  // efficiencies for the benchmarks above
  for( size_t i = 0; i < xbinval.size(); ++i ){
    hpcut->SetBinContent(i+1,pcutgr[5]->Eval(xbinval[i]));
    hpicut->SetBinContent(i+1,picutgr[5]->Eval(xbinval[i]));
    hkcut->SetBinContent(i+1,kcutgr[5]->Eval(xbinval[i]));

    hkpfake_eff->SetBinContent(i+1,kpgr[5]->Eval(xbinval[i]));
    hkpifake_eff->SetBinContent(i+1,kpigr[5]->Eval(xbinval[i]));
    hpikfake_eff->SetBinContent(i+1,pikgr[5]->Eval(xbinval[i]));
    hpimufake_eff->SetBinContent(i+1,pimugr[5]->Eval(xbinval[i]));
    hpiefake_eff->SetBinContent(i+1,piegr[5]->Eval(xbinval[i]));

    hpikfake_eff->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hpimufake_eff->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hpiefake_eff->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hkpifake_eff->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hkpfake_eff->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());

    hpcut->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hpicut->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
    hkcut->GetXaxis()->SetBinLabel(i+1,xlabel[i].c_str());
  }
  
  hpcut->SetXTitle("Proton efficiency");
  hpicut->SetXTitle("Pion efficiency");
  hkcut->SetXTitle("Kaon efficiency");

  hpcut->SetYTitle("PIDp cut value");
  hpicut->SetYTitle("PIDpi cut value");
  hkcut->SetYTitle("PIDk cut value");

  hpikfake_eff->SetXTitle("Pion efficiency");
  hpiefake_eff->SetXTitle("Pion efficiency");
  hpimufake_eff->SetXTitle("Pion efficiency");
  hkpifake_eff->SetXTitle("Kaon efficiency");
  hkpfake_eff->SetXTitle("Kaon efficiency");

  hpiefake_eff->SetYTitle("Electron fake rate");
  hpimufake_eff->SetYTitle("Muon fake rate");
  hpikfake_eff->SetYTitle("Kaon fake rate");
  hkpifake_eff->SetYTitle("Pion fake rate");
  hkpfake_eff->SetYTitle("Proton fake rate");

  hpcut->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hpicut->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hkcut->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));

  hpikfake_eff->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hpimufake_eff->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hpiefake_eff->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hkpifake_eff->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));
  hkpfake_eff->GetListOfFunctions()->Add(new TNamed("Meta", "nostats"));

  hpcut->GetListOfFunctions()->Add(new TNamed("Description", hpcut->GetTitle()));
  hpicut->GetListOfFunctions()->Add(new TNamed("Description", hpicut->GetTitle()));
  hkcut->GetListOfFunctions()->Add(new TNamed("Description", hkcut->GetTitle()));

  hpikfake_eff->GetListOfFunctions()->Add(new TNamed("Description", hpikfake_eff->GetTitle()));
  hpimufake_eff->GetListOfFunctions()->Add(new TNamed("Description", hpimufake_eff->GetTitle()));
  hpiefake_eff->GetListOfFunctions()->Add(new TNamed("Description", hpiefake_eff->GetTitle()));
  hkpifake_eff->GetListOfFunctions()->Add(new TNamed("Description", hkpifake_eff->GetTitle()));
  hkpfake_eff->GetListOfFunctions()->Add(new TNamed("Description", hkpfake_eff->GetTitle()));

  hpcut->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hpicut->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hkcut->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));

  hpikfake_eff->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hpimufake_eff->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hpiefake_eff->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hkpifake_eff->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));
  hkpfake_eff->GetListOfFunctions()->Add(new TNamed("Check", "Consistency between the different histograms"));

  hpcut->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hpicut->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hkcut->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));

  hpikfake_eff->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hpimufake_eff->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hpiefake_eff->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hkpifake_eff->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));
  hkpfake_eff->GetListOfFunctions()->Add(new TNamed("Contact","jan.strube@desy.de"));

  hpcut->Write();
  // hpcut->SaveAs("hpcut_hadrons.pdf");
  hpicut->Write();
  // hpicut->SaveAs("hpicut_hadrons.pdf");
  hkcut->Write();
  // hkcut->SaveAs("hkcut_hadrons.pdf");

  hpikfake_eff->Write();
  // hpikfake_eff->SaveAs("hppifake_hadrons.pdf");
  hpimufake_eff->Write();
  // hpimufake_eff->SaveAs("hpkfake_hadrons.pdf");
  hpiefake_eff->Write();
  // hpiefake_eff->SaveAs("hpkfake_hadrons.pdf");
  hkpifake_eff->Write();
  // hkpifake_eff->SaveAs("hpikfake_hadrons.pdf");
  hkpfake_eff->Write();
  // hkpfake_eff->SaveAs("hkpifake_hadrons.pdf");

  outputFile->Close();
}

void test2_Validation_StandardParticles_ChargedPID(){

  // pion, kaon, and proton samples
  TString pikfile("../ana-dstars.root");
  TString pfile("../ana-xi-lambda.root");

  TFile* pikSample = new TFile(pikfile);
  TTree* piktree = (TTree*)pikSample->Get("dsttree");
  TFile* pSample = new TFile(pfile);
  TTree* ptree = (TTree*)pSample->Get("xitree");

  plotROC(pSample, ptree, pikSample, piktree);
}

