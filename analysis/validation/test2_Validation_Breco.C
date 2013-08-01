////////////////////////////////////////////////////////////
// Validation_Breco.cc
// Reconstruction some benchmark B decay modes
//
// Determine the full Breco efficiency
//
// Note that MC matching is not available for neutral particles
// and full decay matching is yet to be implemented.
//
// NtupleMaker ntuples were used for this study
// 
// Phillip Urquijo
// May 3, 2013
//
gSystem->Load("libRooFit.so");
using namespace RooFit ;
void test2_Validation_B2Dpi();
void test2_Validation_B2Kstgamma();
void test2_Validation_B2JpsiKS();
void test2_Validation_Breco(){

  test2_Validation_B2Dpi();  
  test2_Validation_B2Kstgamma();  
  test2_Validation_B2JpsiKS();  

}
void test2_Validation_B2Dpi(){
  /*  Take the BtoDpi prepared by the NtupleMaker */
  TChain * recoTree = new TChain("Bu_Dpi_tuple");
  recoTree->AddFile("../Bu_Dpi,Kpi.ntup.root");

 TFile * output = TFile::Open("Bu_D0pi,Kpi_Breco.root", "recreate");
 
  TH1F * h_mbc              = new TH1F("mbc","B2Dpi;m_{bc} [GeV];Events",25,5.24,5.29);
  TH1F * h_mbctruthmatch    = new TH1F("mbctruthmatch","B2Dpi;m_{bc} [GeV] (truthmatch);Events",25,5.24,5.29);
  TH1F * h_deltae           = new TH1F("deltae","B2Dpi;#Delta E [GeV];Events",20,-0.2,0.2);
  TH1F * h_deltaetruthmatch = new TH1F("deltaetruthmatch","B2Dpi;#Delta E [GeV] (truth matched);Events",20,-0.2,0.2);

  TH1F * h_md0              = new TH1F("md0","md0;m_{D^{0}} GeV;Events",30,1.84,1.9);

  enum hypo {pion=0, kaon=1, electron=2, muon=3, proton=4, bplus=5, d0=6};
  const int pid[] = {211,321,11,13,2212,521,421};


  float fB_P4[4];  
  float fB_D0_P4[4];  
  float fB_pi_P4[4];  
  float fB_D0_K_P4[4];  
  float fB_D0_pi_P4[4];  

  float fB_mbc;  
  float fB_deltae;  

  int iB_pi_TruthID;  
  int iB_D0_K_TruthID;  
  int iB_D0_pi_TruthID;  

  int iB_pi_MC_MOTHER_ID;  
  int iB_D0_K_MC_MOTHER_ID;  
  int iB_D0_pi_MC_MOTHER_ID;  

  int iB_D0_K_MC_GD_MOTHER_ID;  
  int iB_D0_pi_MC_GD_MOTHER_ID;  

  int inCands;  
  int iiCand;  
  
  recoTree->SetBranchAddress("B_P4",       &fB_P4);  
  recoTree->SetBranchAddress("B_D0_P4",    &fB_D0_P4);  
  recoTree->SetBranchAddress("B_pi_P4",    &fB_pi_P4);  
  recoTree->SetBranchAddress("B_D0_K_P4",  &fB_D0_K_P4);  
  recoTree->SetBranchAddress("B_D0_pi_P4", &fB_D0_pi_P4);  

  recoTree->SetBranchAddress("B_mbc",     &fB_mbc);  
  recoTree->SetBranchAddress("B_deltae",  &fB_deltae);  
  
  recoTree->SetBranchAddress("B_pi_TruthID",   &iB_pi_TruthID);  
  recoTree->SetBranchAddress("B_D0_K_TruthID",  &iB_D0_K_TruthID);  
  recoTree->SetBranchAddress("B_D0_pi_TruthID", &iB_D0_pi_TruthID);  

  recoTree->SetBranchAddress("B_pi_MC_MOTHER_ID",   &iB_pi_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B_D0_K_MC_MOTHER_ID",  &iB_D0_K_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B_D0_pi_MC_MOTHER_ID", &iB_D0_pi_MC_MOTHER_ID);  

  recoTree->SetBranchAddress("B_D0_K_MC_GD_MOTHER_ID",  &iB_D0_K_MC_GD_MOTHER_ID);  
  recoTree->SetBranchAddress("B_D0_pi_MC_GD_MOTHER_ID", &iB_D0_pi_MC_GD_MOTHER_ID);  
  
  recoTree->SetBranchAddress("nCands", &inCands);  
  recoTree->SetBranchAddress("iCand", &iiCand);  
  
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_B(fB_P4);  
    TLorentzVector lv_B_D0(fB_D0_P4);  
    TLorentzVector lv_B_pi(fB_pi_P4);  
    TLorentzVector lv_B_D0_K(fB_D0_K_P4);  
    TLorentzVector lv_B_D0_pi(fB_D0_pi_P4);  

    h_mbc->Fill(fB_mbc);
    h_deltae->Fill(fB_deltae);
    
    //Simple Truth Match of the Tracks and their mothers
    if(abs(iB_pi_TruthID)!=pid[pion])continue;
    if(abs(iB_D0_K_TruthID)!=pid[kaon])continue;
    if(abs(iB_D0_pi_TruthID)!=pid[pion])continue;

    if(abs(iB_pi_MC_MOTHER_ID)!=pid[bplus])continue;
    if(abs(iB_D0_pi_MC_MOTHER_ID)!=pid[d0])continue;
    if(abs(iB_D0_pi_MC_GD_MOTHER_ID)!=pid[bplus])continue;
    if(abs(iB_D0_K_MC_MOTHER_ID)!=pid[d0])continue;
    if(abs(iB_D0_K_MC_GD_MOTHER_ID)!=pid[bplus])continue;

    h_mbctruthmatch->Fill(fB_mbc);
    h_deltaetruthmatch->Fill(fB_deltae);
    h_md0->Fill(lv_B_D0.M());
  }
  
  TCanvas *tc = new TCanvas ("tc","tcReco",1000,800);
  tc->Divide(2,2);
  tc->cd(1);
  h_mbc->Draw();
  h_mbctruthmatch->SetLineColor(kRed);
  h_mbctruthmatch->Draw("same");
  tc->cd(2);
  h_deltae->Draw();
  h_deltaetruthmatch->SetLineColor(kRed);
  h_deltaetruthmatch->Draw("same");

  tc->cd(3);
  h_md0->Draw();


  //Determine the D0 mass resolution

  RooRealVar *mass  =  new RooRealVar("mass","m(D0) GeV" , 1.84, 1.9);
  RooDataHist hmd0("hmd0","hmd0",*mass, h_md0);
  //D0 signal PDF is a Gaussian
  RooRealVar mean("mean","mean",1.867,1.86,1.88);
  RooRealVar sig1("#sigma","sig", 0.005, 0.001,0.009);
  RooGaussian sigcb("sigcb","gau1",*mass,mean,sig1);
  RooArgList shapes;
  shapes.add(sigcb);        
  RooRealVar nsig("Nsig","Nsig", 100,0,1000000);
  RooArgList yields;
  yields.add(nsig);        
  RooAddPdf totalPdf("totalpdf", "",shapes, yields);
  
  tc->cd(4);
  totalPdf->fitTo(hmd0,RooFit::Extended(kTRUE),Minos(1));
  /* Fit to the mass */
  RooPlot *framex = mass->frame();
  hmd0.plotOn(framex, Binning(30),Name("Hist"));
  framex->SetMaximum(framex->GetMaximum());
  totalPdf.plotOn(framex,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
  totalPdf.plotOn(framex,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.paramOn(framex,Parameters(RooArgSet(sig1,mean,nsig)), Format("NELU", AutoPrecision(2)), Layout(0.4, 0.95, 0.95) );
  framex->getAttText()->SetTextSize(0.03); 
  framex->SetMaximum(h_md0->GetMaximum()*1.5);

  float meancut   = mean.getVal(); 
  float meanerror = mean.getError(); 
  float width = sig1.getVal();
  float widtherror = sig1.getError();

  framex->Draw("");

  double nreco           = h_mbc->Integral();
  double nrecotruthmatch = h_mbctruthmatch->Integral();
  double errtm           = sqrt(nrecotruthmatch);
  cout<<"N Reco truth matched= "<<nrecotruthmatch<<endl;
  cout<<"N Reco total = "       <<nreco<<endl;
  double efftrack = pow( float(nrecotruthmatch/1000.), float(1./3.));
  double erreff = errtm/nrecotruthmatch * efftrack * 1/3;

  cout<< "Average Efficiency per track = " << Form("%4.3f +/- %4.3f",efftrack, erreff)<< endl;

  TNtuple* tbreco = new TNtuple("Bu_D0pi reco", "tree", "nreco:d0mean:d0mean_err:d0width:d0width_err" );
  tbreco->Fill(nreco,meancut,meanerror,width,widtherror);

  output->Write();
  delete output;
 
}


void test2_Validation_B2Kstgamma(){
  /*  Take the BtoDpi prepared by the NtupleMaker */
  TChain * recoTree = new TChain("Bd_Kstgamma_tuple");
  recoTree->AddFile("../Bd_Kstgamma.ntup.root");
  TFile * output = TFile::Open("Bd_Kstgamma_Breco.root", "recreate");
 
  TH1F * h_mbc           = new TH1F("mbc","B2Kstgamma;m_{bc} [GeV];Events",25,5.24,5.29);
  TH1F * h_mbctruthmatch = new TH1F("mbctruthmatch","B2Kstgamma;m_{bc} [GeV] (truth matched);Events",25,5.24,5.29);
  TH1F * h_deltae           = new TH1F("deltae","B2Kstgamma;#Delta E [GeV];Events",20,-0.2,0.2);
  TH1F * h_deltaetruthmatch = new TH1F("deltaetruthamatch","B2Kstgamma;#Delta E [GeV] (truth matched);Events",20,-0.2,0.2);

  TH1F * h_egamma         = new TH1F("egamma","B2Kstgamma;E_{#gamma} [GeV];Events",17,0.,3.4);
  TH1F * h_egammatruthmatch         = new TH1F("egammatruthmatch","B2Kstgamma;E_{#gamma} [GeV] (truth matched);Events",17,0.,3.4);
  TH1F * h_mkst           = new TH1F("mkst","B2Kstgamma;m_{K^{*0}} [GeV];Events",25,0.5,1.5);
  TH1F * h_mksttruthmatch           = new TH1F("mksttruthmatch","B2Kstgamma;m_{K^{*0}} [GeV] (truth matched);Events",25,0.5,1.5);

  enum hypo {pion=0, kaon=1, electron=2, muon=3, proton=4, bplus=5, d0=6, photon=7, bzero=8, kstar=9};
  const int pid[] = {211,321,11,13,2212,521,421,22,511,313};


  float fB0_P4[4];  
  float fB0_KST0_P4[4];  
  float fB0_gamma_P4[4];  
  float fB0_KST0_K_P4[4];  
  float fB0_KST0_pi_P4[4];  

  float fB0_mbc;  
  float fB0_deltae;  

  float fB0_KST0_K_PIDk;  

  int iB0_gamma_TruthID;  
  int iB0_KST0_K_TruthID;  
  int iB0_KST0_pi_TruthID;  

  int iB0_gamma_MC_MOTHER_ID;  
  int iB0_KST0_K_MC_MOTHER_ID;  
  int iB0_KST0_pi_MC_MOTHER_ID;  

  int iB0_KST0_K_MC_GD_MOTHER_ID;  
  int iB0_KST0_pi_MC_GD_MOTHER_ID;  

  int inCands;  
  int iiCand;  
  
  recoTree->SetBranchAddress("B0_P4",      &fB0_P4);  
  recoTree->SetBranchAddress("B0_KST0_P4",    &fB0_KST0_P4);  
  recoTree->SetBranchAddress("B0_gamma_P4",   &fB0_gamma_P4);  
  recoTree->SetBranchAddress("B0_KST0_K_P4",  &fB0_KST0_K_P4);  
  recoTree->SetBranchAddress("B0_KST0_pi_P4", &fB0_KST0_pi_P4);  

  recoTree->SetBranchAddress("B0_mbc",     &fB0_mbc);  
  recoTree->SetBranchAddress("B0_deltae",  &fB0_deltae);  

  recoTree->SetBranchAddress("B0_KST0_K_PIDk",     &fB0_KST0_K_PIDk);  
  
  recoTree->SetBranchAddress("B0_gamma_TruthID",   &iB0_gamma_TruthID);  
  recoTree->SetBranchAddress("B0_KST0_K_TruthID",  &iB0_KST0_K_TruthID);  
  recoTree->SetBranchAddress("B0_KST0_pi_TruthID", &iB0_KST0_pi_TruthID);  

  recoTree->SetBranchAddress("B0_gamma_MC_MOTHER_ID",   &iB0_gamma_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KST0_K_MC_MOTHER_ID",  &iB0_KST0_K_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KST0_pi_MC_MOTHER_ID", &iB0_KST0_pi_MC_MOTHER_ID);  

  recoTree->SetBranchAddress("B0_KST0_K_MC_GD_MOTHER_ID",  &iB0_KST0_K_MC_GD_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KST0_pi_MC_GD_MOTHER_ID", &iB0_KST0_pi_MC_GD_MOTHER_ID);  
  
  recoTree->SetBranchAddress("nCands", &inCands);  
  recoTree->SetBranchAddress("iCand", &iiCand);  
  
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_B(fB0_P4);  
    TLorentzVector lv_B0_KST0(fB0_KST0_P4);  
    TLorentzVector lv_B0_gamma(fB0_gamma_P4);  
    TLorentzVector lv_B0_KST0_K(fB0_KST0_K_P4);  
    TLorentzVector lv_B0_KST0_pi(fB0_KST0_pi_P4);  

    //PID requirement
     if(fB0_KST0_K_PIDk<0.5)continue;
    //Select photons above 50 MeV
    if(lv_B0_gamma.E()<0.05)continue;
    //Select the K* Kpi mass range
    if(fabs(lv_B0_KST0.M()-0.892)>0.15)continue;
    h_mkst->Fill(lv_B0_KST0.M());
    
    h_mbc->Fill(fB0_mbc);
    h_deltae->Fill(fB0_deltae);
    h_egamma->Fill(lv_B0_gamma.E());


    //Simple Truth Match of the Tracks and their mothers
    //if(abs(iB0_gamma_TruthID)!=pid[photon])continue;
    if(abs(iB0_KST0_K_TruthID)!=pid[kaon])continue;
    if(abs(iB0_KST0_pi_TruthID)!=pid[pion])continue;

    //if(abs(iB0_gamma_MC_MOTHER_ID)!=pid[bzero])continue;
    if(abs(iB0_KST0_pi_MC_MOTHER_ID)!=pid[kstar])continue;
    if(abs(iB0_KST0_pi_MC_GD_MOTHER_ID)!=pid[bzero])continue;
    if(abs(iB0_KST0_K_MC_MOTHER_ID)!=pid[kstar])continue;
    if(abs(iB0_KST0_K_MC_GD_MOTHER_ID)!=pid[bzero])continue;

    h_mbctruthmatch->Fill(fB0_mbc);
    h_deltaetruthmatch->Fill(fB0_deltae);
    h_egammatruthmatch->Fill(lv_B0_gamma.E());
    h_mksttruthmatch->Fill(lv_B0_KST0.M());
  }
  
  TCanvas *tc = new TCanvas ("tc","tcReco",1600,1000);
  tc->Divide(2,2);
  tc->cd(1);
  h_mbc->Draw();
  h_mbctruthmatch->SetLineColor(kRed);
  h_mbctruthmatch->Draw("same");
  tc->cd(2);
  h_deltae->Draw();
  h_deltaetruthmatch->SetLineColor(kRed);
  h_deltaetruthmatch->Draw("same");
  tc->cd(3);
  h_egamma->Draw();
  h_egammatruthmatch->SetLineColor(kRed);
  h_egammatruthmatch->Draw("same");
  tc->cd(4);
  h_mkst->Draw();
  h_mksttruthmatch->SetLineColor(kRed);
  h_mksttruthmatch->Draw("same");
  
  double nreco           = h_mbc->Integral();
  double nrecotruthmatch = h_mbctruthmatch->Integral();
  double errtm           = sqrt(nrecotruthmatch);
  cout<<"N Reco truth matched= "<<nrecotruthmatch<<endl;
  cout<<"N Reco total = "       <<nreco<<endl;
  double efftrack = pow( float(nrecotruthmatch/1000.), float(1./3.));
  double erreff = errtm/nrecotruthmatch * efftrack * 1/3;

  cout<< "Average Efficiency per particle = " << Form("%4.3f +/- %4.3f",efftrack, erreff)<< endl;
 
  TNtuple* tbreco = new TNtuple("Bd_K*gamma reco", "tree", "nreco" );
  tbreco->Fill(nreco);
  
  output->Write();
  delete output;
 
}

void test2_Validation_B2JpsiKS(){
  /*  Take the BtoDpi prepared by the NtupleMaker */
  TChain * recoTree = new TChain("Bd_JpsiKS_tuple");
  recoTree->AddFile("../Bd_JpsiKS,mumu.ntup.root");
   TFile * output = TFile::Open("Bd_JpsiKS,mumu_Breco.root", "recreate");
 
  TH1F * h_mbc              = new TH1F("mbc","B2JpsiKS;m_{bc} [GeV];Events"                       ,25,5.24,5.29);
  TH1F * h_mbctruthmatch    = new TH1F("mbctruthmatch","B2JpsiKS;m_{bc} [GeV] (truthmatch);Events",25,5.24,5.29);
  TH1F * h_deltae           = new TH1F("deltae","B2JpsiKS;#Delta E [GeV];Events"                          ,20,-0.2,0.2);
  TH1F * h_deltaetruthmatch = new TH1F("deltaetruthmatch","B2JpsiKS;#Delta E [GeV] (truth matched);Events",20,-0.2,0.2);

  TH1F * h_mjpsi            = new TH1F("mjpsi","B2JpsiKS;m_{J/#psi} [GeV];Events",20,3.,3.2);
  TH1F * h_mjpsitruthmatch  = new TH1F("mjpsitruthmatch","B2JpsiKS;m_{J/#psi} [GeV] (truth matched);Events",20,3.,3.2);
  TH1F * h_mks              = new TH1F("mks","B2JpsiKS;m_{KS} [GeV];Events",20,0.4,0.6);
  TH1F * h_mkstruthmatch    = new TH1F("mkstruthmatch","B2JpsiKS;m_{KS} [GeV] (truth matched);Events",20,0.4,0.6);

  enum hypo {pion=0, kaon=1, electron=2, muon=3, proton=4, bplus=5, d0=6, photon=7, bzero=8, kstar=9, ks=10, jpsi=11};
  const int pid[] = {211,321,11,13,2212,521,421,22,511,313,310,443};


  float fB0_P4[4];  
  float fB0_KS0_P4[4];  
  float fB0_JPsi_P4[4];  
  float fB0_KS0_pi0_P4[4];  
  float fB0_KS0_pi1_P4[4];  

  float fB0_mbc;  
  float fB0_deltae;  

  int iB0_JPsi_mu0_TruthID;  
  int iB0_JPsi_mu1_TruthID;  
  int iB0_KS0_pi0_TruthID;  
  int iB0_KS0_pi1_TruthID;  

  int iB0_JPsi_mu0_MC_MOTHER_ID;  
  int iB0_JPsi_mu1_MC_MOTHER_ID;  
  int iB0_KS0_pi0_MC_MOTHER_ID;  
  int iB0_KS0_pi1_MC_MOTHER_ID;  

  int iB0_KS0_pi0_MC_GD_MOTHER_ID;  
  int iB0_KS0_pi1_MC_GD_MOTHER_ID;  

  int inCands;  
  int iiCand;  
  
  recoTree->SetBranchAddress("B0_P4",      &fB0_P4);  
  recoTree->SetBranchAddress("B0_KS0_P4",    &fB0_KS0_P4);  
  recoTree->SetBranchAddress("B0_JPsi_P4",   &fB0_JPsi_P4);  
  recoTree->SetBranchAddress("B0_KS0_pi0_P4",  &fB0_KS0_pi0_P4);  
  recoTree->SetBranchAddress("B0_KS0_pi1_P4", &fB0_KS0_pi1_P4);  

  recoTree->SetBranchAddress("B0_mbc",     &fB0_mbc);  
  recoTree->SetBranchAddress("B0_deltae",  &fB0_deltae);  

  recoTree->SetBranchAddress("B0_JPsi_mu0_TruthID",   &iB0_JPsi_mu0_TruthID);  
  recoTree->SetBranchAddress("B0_JPsi_mu1_TruthID",   &iB0_JPsi_mu1_TruthID);  
  recoTree->SetBranchAddress("B0_KS0_pi0_TruthID",  &iB0_KS0_pi0_TruthID);  
  recoTree->SetBranchAddress("B0_KS0_pi1_TruthID", &iB0_KS0_pi1_TruthID);  

  recoTree->SetBranchAddress("B0_JPsi_mu0_MC_MOTHER_ID",   &iB0_JPsi_mu0_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_JPsi_mu1_MC_MOTHER_ID",   &iB0_JPsi_mu1_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KS0_pi0_MC_MOTHER_ID",  &iB0_KS0_pi0_MC_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KS0_pi1_MC_MOTHER_ID", &iB0_KS0_pi1_MC_MOTHER_ID);  

  recoTree->SetBranchAddress("B0_KS0_pi0_MC_GD_MOTHER_ID",  &iB0_KS0_pi0_MC_GD_MOTHER_ID);  
  recoTree->SetBranchAddress("B0_KS0_pi1_MC_GD_MOTHER_ID", &iB0_KS0_pi1_MC_GD_MOTHER_ID);  
  
  recoTree->SetBranchAddress("nCands", &inCands);  
  recoTree->SetBranchAddress("iCand", &iiCand);  
  
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_B(fB0_P4);  
    TLorentzVector lv_B0_KS0(fB0_KS0_P4);  
    TLorentzVector lv_B0_JPsi(fB0_JPsi_P4);  
    TLorentzVector lv_B0_KS0_K(fB0_KS0_pi0_P4);  
    TLorentzVector lv_B0_KS0_pi(fB0_KS0_pi1_P4);  

    //Select Jpsi mass range
    if((lv_B0_JPsi.M()-3.1)>0.05)continue;
    //Select the KS mass range
    if(fabs(lv_B0_KS0.M()-0.492)>0.05)continue;
    h_mks->Fill(lv_B0_KS0.M());
    
    h_mbc->Fill(fB0_mbc);
    h_deltae->Fill(fB0_deltae);
    h_mjpsi->Fill(lv_B0_JPsi.M());


    //Simple Truth Match of the Tracks and their mothers
    if(abs(iB0_JPsi_mu0_TruthID)!=pid[muon])continue;
    if(abs(iB0_JPsi_mu1_TruthID)!=pid[muon])continue;
    if(abs(iB0_KS0_pi0_TruthID)!=pid[pion])continue;
    if(abs(iB0_KS0_pi1_TruthID)!=pid[pion])continue;

    if(abs(iB0_JPsi_mu0_MC_MOTHER_ID)!=pid[jpsi])continue;
    if(abs(iB0_JPsi_mu1_MC_MOTHER_ID)!=pid[jpsi])continue;
    if(abs(iB0_KS0_pi1_MC_MOTHER_ID)!=pid[ks])continue;
    if(abs(iB0_KS0_pi1_MC_GD_MOTHER_ID)!=pid[bzero])continue;
    if(abs(iB0_KS0_pi0_MC_MOTHER_ID)!=pid[ks])continue;
    if(abs(iB0_KS0_pi0_MC_GD_MOTHER_ID)!=pid[bzero])continue;

    h_mbctruthmatch->Fill(fB0_mbc);
    h_deltaetruthmatch->Fill(fB0_deltae);
    h_mjpsitruthmatch->Fill(lv_B0_JPsi.M());
    h_mkstruthmatch->Fill(lv_B0_KS0.M());
  }
  
  TCanvas *tc = new TCanvas ("tc","tcReco",1600,1000);
  tc->Divide(3,2);
  tc->cd(1);
  h_mbc->Draw();
  h_mbctruthmatch->SetLineColor(kRed);
  h_mbctruthmatch->Draw("same");
  tc->cd(2);
  h_deltae->Draw();
  h_deltaetruthmatch->SetLineColor(kRed);
  h_deltaetruthmatch->Draw("same");
  tc->cd(3);
  h_mjpsi->Draw();
  h_mjpsitruthmatch->SetLineColor(kRed);
  h_mjpsitruthmatch->Draw("same");
  tc->cd(4);
  h_mks->Draw();
  h_mkstruthmatch->SetLineColor(kRed);
  h_mkstruthmatch->Draw("same");
  

  //Determine the jpsi mass resolution

  RooRealVar *mass  =  new RooRealVar("mass","m(J/#psi) GeV" , 3.0, 3.2);
  RooDataHist hpsi("hpsi","hpsi",*mass, h_mjpsi);
  //J/Psi signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
  RooRealVar mean("mean","mean",3.1,3.0,3.2);
  RooRealVar sig1("#sigma","sig", 0.6, 0.001,1.);
  //RooGaussian sigcb("sigcb","gau1",*mass,mean,sig1);
  RooBreitWigner sigcb("sigcb","gau1",*mass,mean,sig1);

  RooRealVar alphacb("alphacb", "alpha", 1.4,0.1,5.8);
  RooRealVar ncb("ncb", "n", 8,1.,15);
  //RooCBShape sigcb("sigcb","sig",*mass,mean,sig1,alphacb,ncb);
  
  RooArgList shapes;
  //shapes.add(gau1);        
  shapes.add(sigcb);        

  // Perform the extended fit to get a rough idea of yields.
  RooRealVar nsig("Nsig","Nsig", 100,0,1000000);
  RooRealVar nbkg("Nbkg","Nbkg", 100,0,1000000);
  RooArgList yields;
  yields.add(nsig);        
  
  RooAddPdf totalPdf("totalpdf", "",shapes, yields);
  
  tc->cd(5);
  totalPdf->fitTo(hpsi,RooFit::Extended(kTRUE),Minos(1));
  /* Fit to the unfit mass with cuts */
  RooPlot *framex = mass->frame();
  hpsi.plotOn(framex, Binning(50),Name("Hist"));
  framex->SetMaximum(framex->GetMaximum());
  totalPdf.plotOn(framex,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
  //totalPdf.plotOn(framex,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.plotOn(framex,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
  totalPdf.paramOn(framex,Parameters(RooArgSet(sig1,mean,nsig)), Format("NELU", AutoPrecision(2)), Layout(0.4, 0.95, 0.95) );
  framex->getAttText()->SetTextSize(0.03); 
  framex->SetMaximum(h_mjpsi->GetMaximum()*1.5);

  float meancut   = mean.getVal(); 
  float meanerror = mean.getError(); 
  float width      = sig1.getVal();
  float widtherror = sig1.getError();

  framex->Draw("");


  double nreco           = h_mbc->Integral();
  double nrecotruthmatch = h_mbctruthmatch->Integral();
  double errtm           = sqrt(nrecotruthmatch);
  cout<<"N Reco truth matched= "<<nrecotruthmatch<<endl;
  cout<<"N Reco total = "       <<nreco<<endl;
  double efftrack = pow( float(nrecotruthmatch/1000.), float(1./4.));
  double erreff   = errtm/nrecotruthmatch * efftrack * 1/4;

  cout<< "Average Efficiency per track = " << Form("%4.3f +/- %4.3f",efftrack, erreff)<< endl;
 
  TNtuple* tbreco = new TNtuple("Bd_JpsiKS,mumu reco", "tree", "nreco:mean:meanerror:width:widtherror" );
  tbreco->Fill(nreco, meancut,meanerror,width,widtherror);

  output->Write();
  delete output;
 
}
