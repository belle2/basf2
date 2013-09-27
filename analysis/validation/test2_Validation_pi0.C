////////////////////////////////////////////////////////////
// Validation_pi0.cc
// Check the calibration of the ECL in the MC by 
// determining the measured pi0 invariant mass
// Fit parameter constraints need some tuning.
// 
// Note: MC matching is not currently available in the ECL
//
// Phillip Urquijo
// May 3, 2013
//

gSystem->Load("libRooFit.so");
using namespace RooFit ;
void test2_Validation_pi0(){
 
  /*  Take the pi0tuple prepared by the NtupleMaker */
  TChain * recoTree = new TChain("pi0tuple");
  recoTree->AddFile("../GenericB.ntup.root");

  //Plots used in offline validation
  /* Invariant mass after Egamma>0.05 GeV criterion */
  TH1F * h_pi0_mcut = new TH1F("mpi0cut","#pi^{0} mass with photon cut;m(#pi^{0}) [GeV];N",40,0.08,0.18);

  //Plots for online/web validation
  TFile* output = new TFile("pi0Validation.root", "recreate");
  /* Mass constrained fit value, as stored in Particle */
  TH1F * h_pi0_mf   = new TH1F("mpi0f",";Mass constrained fit m(#pi^{0}) [GeV];N",40,0.133,0.137);
  h_pi0_mf->GetListOfFunctions()->Add(new TNamed("Description", "pi0 Mass constrained fit mass, with background. A Generic BBbar sample is used. Test may be replaced with analysis mode validation with pi0."));
  h_pi0_mf->GetListOfFunctions()->Add(new TNamed("Check", "Stable S/B, non-empty (i.e. pi0 import to analysis modules is working), consistent mean."));

  /* Invariant mass determined from the two photon daughters */
  TH1F * h_pi0_m    = new TH1F("mpi0","#pi^{0} mass no cut;m(#pi^{0}) [GeV];N",40,0.08,0.18);
  h_pi0_m->GetListOfFunctions()->Add(new TNamed("Description", "pi0 Mass, with background. A Generic BBbar sample is used. Test may be replaced with analysis mode validation with pi0."));
  h_pi0_m->GetListOfFunctions()->Add(new TNamed("Check", "Stable S/B, non-empty (i.e. pi0 import to analysis modules is working), consistent mean."));

  /* Access the Photons and pi0 M*/
  float fpi0_gamma0_P4[4];  
  float fpi0_gamma1_P4[4];  
  float fpi0_M;  
  recoTree->SetBranchAddress("pi0_gamma0_P4", &fpi0_gamma0_P4);  
  recoTree->SetBranchAddress("pi0_gamma1_P4", &fpi0_gamma1_P4);  
  recoTree->SetBranchAddress("pi0_M", &fpi0_M);  

  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_pi0_gamma0(fpi0_gamma0_P4);  
    TLorentzVector lv_pi0_gamma1(fpi0_gamma1_P4);  
    TLorentzVector lv_pi0_raw = lv_pi0_gamma0+lv_pi0_gamma1;
    float fpi0_raw_M = lv_pi0_raw.M(); 
    h_pi0_mf->Fill(fpi0_M);
    h_pi0_m->Fill(fpi0_raw_M);
    if(lv_pi0_gamma0.E()>0.05&&lv_pi0_gamma1.E()>0.05)h_pi0_mcut->Fill(fpi0_raw_M);
  }

  TCanvas *tc = new TCanvas ("tcReco","tcReco",600,600);
  tc->Print("pi0.pdf[");

  TCanvas *tc = new TCanvas ("tcReco","tcReco",600,600);
  // Mass constrained fit mass
  h_pi0_mf->SetLineColor(kRed);
  h_pi0_mf->SetMinimum(0.);
  h_pi0_mf->Draw();
  tc->Print("pi0.pdf");

  // Raw masses (unfit)
  h_pi0_m->SetLineColor(kRed);
  h_pi0_m->Draw();
  h_pi0_m->SetMinimum(0.);
  h_pi0_mcut->SetLineColor(kBlue);
  h_pi0_mcut->Draw("same");
  tc->Print("pi0.pdf");

  RooRealVar *mass  =  new RooRealVar("mass","m(#pi^{0}) GeV" , 0.08, 0.18);
  RooDataHist hpicut("hpicut","hpicut",*mass, h_pi0_mcut);
  RooDataHist hpinocut("hpinocut","hpinocut",*mass, h_pi0_m);

  //pi0 signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
  RooRealVar mean("mean","mean",0.14,0.13,0.16);
  RooRealVar sig1("#sigma","sig", 0.6, 0.002,1.);
  RooGaussian gau1("gau1","gau1",*mass,mean,sig1);

  RooRealVar alphacb("alphacb", "alpha", 1.4,0.1,1.8);
  RooRealVar ncb("ncb", "n", 8,2.,15);
  RooCBShape sigcb("sigcb","sig",*mass,mean,sig1,alphacb,ncb);

  //pi0 background PDF is a 2nd order Chebyshev
  RooRealVar b1("b1","b1",-3.0021e-01,-3,3);
  RooRealVar a1("a1","a1",-3.0021e-01,-3,3);
  RooArgList bList(a1,b1);
  RooChebychev bkg("bkg","pol 2", *mass, bList);
  
  RooArgList shapes;
  //  shapes.add(gau1);        
  shapes.add(sigcb);        
  shapes.add(bkg);

  // Perform the extended fit to get a rough idea of yields.
  RooRealVar nsig("Nsig","Nsig", 100,0,1000000);
  RooRealVar nbkg("Nbkg","Nbkg", 100,0,1000000);
  RooArgList yields;
  yields.add(nsig);        
  yields.add(nbkg);
  
  RooAddPdf totalPdf("totalpdf", "",shapes, yields);
  
  totalPdf->fitTo(hpicut,RooFit::Extended(kTRUE),Minos(1));
  /* Fit to the unfit mass with cuts */
  RooPlot *framex = mass->frame();
  hpicut.plotOn(framex, Binning(40),Name("Hist"));
  framex->SetMaximum(framex->GetMaximum());
  totalPdf.plotOn(framex,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
  //  totalPdf.plotOn(framex,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.plotOn(framex,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.plotOn(framex,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.paramOn(framex,Parameters(RooArgSet(sig1,mean,nsig)), Format("NELU", AutoPrecision(2)), Layout(0.4, 0.95, 0.95) );
  framex->getAttText()->SetTextSize(0.03); 
  framex->SetMaximum(h_pi0_mcut->GetMaximum()*1.5);

  float meancut   = mean->getVal(); 
  float meanerror = mean->getError(); 
  float width = sig1.getVal();
  float widtherror = sig1.getError();

  framex->Draw("");
  tc->Print("pi0.pdf");
  
  /* Fit to the unfit mass with cuts */
  totalPdf->fitTo(hpinocut,RooFit::Extended(kTRUE),Minos(1));
  RooPlot *framey = mass->frame();
  hpinocut.plotOn(framey, Binning(40),Name("Hist"));
  framey->SetMaximum(framey->GetMaximum());
  totalPdf.plotOn(framey,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
  //  totalPdf.plotOn(framey,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.plotOn(framey,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.plotOn(framey,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.paramOn(framey,Parameters(RooArgSet(sig1,mean,nsig)), Format("NELU", AutoPrecision(2)), Layout(0.4, 0.95, 0.95) );
  framey->getAttText()->SetTextSize(0.03); 
  framey->SetMaximum(h_pi0_m->GetMaximum()*1.5);
  framey->Draw();
  tc->Print("pi0.pdf");
  
  /* Save the numerical fit results to a validation ntuple */
  string namelist;
  TNtuple* tvalidation = new TNtuple("pi0mass", "tree", "mean:meanerror:width:widtherror");
  tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);
  tvalidation->SetAlias("Description", "Fit to the pi0 mass in background conditions. Note this test may be replaced due to overlap with ECL specific tests.");
  tvalidation->SetAlias("Check", "Consistent numerical fit results.");

  tc->Print("pi0.pdf");
  tc->Print("pi0.pdf]");

  output->Write();
  //delete output;



}

