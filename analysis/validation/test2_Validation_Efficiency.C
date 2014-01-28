////////////////////////////////////////////////////////////
// Validation_Efficiency.cc
// Check the Reconstruction efficiency for single particles
// 
// Phillip Urquijo
// July, 2013
//

enum hypo {pion=0};
const int pid[] = {211};
const float pdgmasses[] = {0.13957};
const char *names[] = { "pi"};

//Lab momentum
TH1F * h_PpiTruth ;
//Lab cos(theta)
TH1F * h_ThpiTruthLowpt ;
TH1F * h_ThpiTruthHighpt ;
//Lab phi
TH1F * h_PhpiTruthLowpt ;
TH1F * h_PhpiTruthHighpt ;

TH1F * h_Ppi ;
TH1F * h_ThpiLowpt ;
TH1F * h_ThpiHighpt ;
TH1F * h_PhpiLowpt ;
TH1F * h_PhpiHighpt ;

//Photons
TH1F * h_PgammaTruth ;
TH1F * h_ThgammaTruthLowE ;
TH1F * h_ThgammaTruthHighE ;
TH1F * h_PhgammaTruthLowE ;
TH1F * h_PhgammaTruthHighE ;

TH1F * h_Pgamma ;
TH1F * h_ThgammaLowE ;
TH1F * h_ThgammaHighE ;
TH1F * h_PhgammaLowE ;
TH1F * h_PhgammaHighE ;

void test2_Validation_Efficiency_Truth(TString);
void test2_Validation_Efficiency_Reco(TString);
void test2_Validation_Efficiency_Photon_Truth(TString, int);
void test2_Validation_Efficiency_Photon_Reco(TString, int);

void test2_Validation_Efficiency(int region=0, bool runOffline=false){ // region 0=all, 1=barrel, 2=forward, 3=backward


  //If running offline study then use finer binning
  int binfactor=1;
  if(runOffline)binfactor=2;
  int nthbins=16*binfactor;
  float thlow=0;
  float thhigh=3.2;
  float phlow=-3.2;
  float phhigh=3.2;
  
  int npbins=13;
  float plow=0.;
  float phigh=2.6;
  
  int nthgbins=25;
  float thglow=0.21;
  float thghigh=2.71;

  int nptbins=26*binfactor;
  float ptlow=0.;
  float pthigh=2.6;

  //line for showing forward and background regions of the ECL
  TLine *lfwdlow  = new TLine(0.21,0.,0.21,1.05);
  lfwdlow->SetLineColor(kGray);
  TLine *lfwdhigh = new TLine(0.61,0.,0.61,1.05);
  lfwdhigh->SetLineColor(kGray);
  TLine *lbwdlow  = new TLine(2.21,0.,2.21,1.05);
  lbwdlow->SetLineColor(kGray);
  TLine *lbwdhigh = new TLine(2.71,0.,2.71,1.05);
  lbwdhigh->SetLineColor(kGray);
  
  //Set the Binning
  //Lab momentum
  h_PpiTruth  = new TH1F("P_pi_Truth" ,";p_{T}(#pi) GeV;N" ,nptbins,ptlow,pthigh);
  //Lab cos(theta)
  h_ThpiTruthLowpt  = new TH1F("Theta_pi_Truth_Lowpt", "p_{T}<250 MeV;#theta(#pi) ;N"  ,nthbins,thlow,thhigh);	
  h_ThpiTruthHighpt = new TH1F("Theta_pi_Truth_Highpt","p_{T}>=250 MeV;#theta(#pi) ;N" ,nthbins,thlow,thhigh);	
  //Lab phi
  h_PhpiTruthLowpt  = new TH1F("Phi_pi_Truth_Lowpt", "p_{T}<250 MeV;#phi(#pi) ;N"  ,nthbins,phlow,phhigh);	
  h_PhpiTruthHighpt = new TH1F("Phi_pi_Truth_Highpt","p_{T}>=250 MeV;#phi(#pi) ;N" ,nthbins,phlow,phhigh);	

  h_Ppi        = new TH1F("P_pi_"          ,";p_{T} GeV;N"                  ,nptbins,ptlow,pthigh);
  h_ThpiLowpt  = new TH1F("Theta_pi_Lowpt" ,"p_{T}<250 MeV;#theta(#pi) ;N"  ,nthbins,thlow,thhigh);	
  h_ThpiHighpt = new TH1F("Theta_pi_Highpt","p_{T}>=250 MeV;#theta(#pi) ;N" ,nthbins,thlow,thhigh);	
  h_PhpiLowpt  = new TH1F("Phi_pi_Lowpt"   ,"p_{T}<250 MeV;#phi(#pi) ;N"    ,nthbins,phlow,phhigh);	
  h_PhpiHighpt = new TH1F("Phi_pi_Highpt"  ,"p_{T}>=250 MeV;#phi(#pi) ;N"   ,nthbins,phlow,phhigh);	

//Photons
  h_PgammaTruth        = new TH1F("P_gamma_Truth"         ,";E(#gamma) GeV;N"             ,npbins,plow,phigh);
  h_ThgammaTruthLowE  = new TH1F("Theta_gamma_Truth_LowE" ,"E<500 MeV;#theta(#gamma) ;N"  ,nthgbins,thglow,thghigh);	
  h_ThgammaTruthHighE = new TH1F("Theta_gamma_Truth_HighE","E>=500 MeV;#theta(#gamma) ;N" ,nthgbins,thglow,thghigh);	
  h_PhgammaTruthLowE  = new TH1F("Phi_gamma_Truth_LowE"   ,"E<500 MeV;#phi(#gamma) ;N"    ,nthbins,phlow,phhigh);	
  h_PhgammaTruthHighE = new TH1F("Phi_gamma_Truth_HighE"  ,"E>=500 MeV;#phi(#gamma) ;N"   ,nthbins,phlow,phhigh);	

  h_Pgamma       = new TH1F("P_gamma_"         ,";E(#gamma) GeV;N"   ,npbins,plow,phigh);
  h_ThgammaLowE  = new TH1F("Theta_gamma_LowE" ,";#theta(#gamma) ;N" ,nthgbins,thglow,thghigh);	
  h_ThgammaHighE = new TH1F("Theta_gamma_HighE",";#theta(#gamma) ;N" ,nthgbins,thglow,thghigh);	
  h_PhgammaLowE  = new TH1F("Phi_gamma_LowE"   ,";#phi(#gamma) ;N"   ,nthbins,phlow,phhigh);	
  h_PhgammaHighE = new TH1F("Phi_gamma_HighE"  ,";#phi(#gamma) ;N"   ,nthbins,phlow,phhigh);	

  TString testfile ("../GenericB.ntup.root");

  TPaveText *belleName = new TPaveText(0.6,0.9,0.9,0.95,"BRNDC");
  belleName->SetFillColor(0);
  belleName->SetTextColor(kGray);
  belleName->SetTextAlign(12);
  belleName->SetBorderSize(0);
  belleName->AddText("Belle II Validation");

  test2_Validation_Efficiency_Truth(testfile);
  test2_Validation_Efficiency_Reco(testfile);
  test2_Validation_Efficiency_Photon_Truth(testfile,region);
  test2_Validation_Efficiency_Photon_Reco(testfile,region);

  TCanvas *maincanvas = new TCanvas ("maincanvas","maincanvas");

  ///////////////////// Tracks
  h_PpiTruth->SetMaximum(h_PpiTruth->GetMaximum()*1.2);
  h_PpiTruth->SetMinimum(0.);
  h_PpiTruth->SetLineColor(kBlue);
  h_PpiTruth->Draw();
  h_Ppi->SetLineColor(kRed);
  h_Ppi->SetLineStyle(2);
  h_Ppi->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf(","Title:Track p_{T} truth and reco");

  h_ThpiTruthLowpt->SetMaximum(h_ThpiTruthLowpt->GetMaximum()*1.2);
  h_ThpiTruthLowpt->SetMinimum(0);
  h_ThpiTruthLowpt->SetLineColor(kBlue);
  h_ThpiLowpt->SetLineColor(kRed);
  h_ThpiLowpt->SetLineStyle(2);
  h_ThpiTruthLowpt->Draw();
  h_ThpiLowpt->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track #theta_{lab} truth and reco, pt<250 MeV");

  h_ThpiTruthHighpt->SetMaximum(h_ThpiTruthHighpt->GetMaximum()*1.2);
  h_ThpiTruthHighpt->SetMinimum(0);
  h_ThpiTruthHighpt->SetLineColor(kBlue);
  h_ThpiHighpt->SetLineColor(kRed);
  h_ThpiHighpt->SetLineStyle(2);
  h_ThpiTruthHighpt->Draw();
  h_ThpiHighpt->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track #theta_{lab} truth and reco, pt>=250 MeV");

  h_PhpiTruthLowpt->SetMaximum(h_PhpiTruthLowpt->GetMaximum()*1.2);
  h_PhpiTruthLowpt->SetMinimum(0);
  h_PhpiTruthLowpt->SetLineColor(kBlue);
  h_PhpiLowpt->SetLineColor(kRed);
  h_PhpiLowpt->SetLineStyle(2);
  h_PhpiTruthLowpt->Draw();
  h_PhpiLowpt->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track #phi_{lab} truth and reco, pt<250 MeV");

  h_PhpiTruthHighpt->SetMaximum(h_PhpiTruthHighpt->GetMaximum()*1.2);
  h_PhpiTruthHighpt->SetMinimum(0);
  h_PhpiTruthHighpt->SetLineColor(kBlue);
  h_PhpiHighpt->SetLineColor(kRed);
  h_PhpiHighpt->SetLineStyle(2);
  h_PhpiTruthHighpt->Draw();
  h_PhpiHighpt->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track #phi_{lab} truth and reco, pt>=250 MeV");

  TH1F * h_p  = new TH1F("pdummy","Eff Track;p_{T} GeV; Track Efficiency",10,ptlow,pthigh);

  TH1F * h_th  = new TH1F("thdummylow","Eff Track;#theta ; Track Efficiency",10,thlow,thhigh);
  TH1F * h_ph  = new TH1F("phdummylow","Eff Track;#phi ; Track Efficiency",10,phlow,phhigh);
  maincanvas->cd()->SetGridy(1);
  maincanvas->cd()->SetGridx(1);

  TGraphAsymmErrors *Eff_Track = new TGraphAsymmErrors();
  Eff_Track  -> Divide(h_Ppi ,h_PpiTruth ,"cl=0.683 b(1,1) mode");
  h_p->Draw();
  Eff_Track->SetFillColor(kBlue);
  Eff_Track->SetMarkerColor(kBlue);
  Eff_Track->SetMarkerStyle(21);
  Eff_Track->Draw("2p");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track efficiency p_{T}");

  TGraphAsymmErrors *Eff_TrackThLowpt = new TGraphAsymmErrors();
  Eff_TrackThLowpt  -> Divide(h_ThpiLowpt ,h_ThpiTruthLowpt ,"cl=0.683 b(1,1) mode");
  h_th->Draw();
  Eff_TrackThLowpt->SetFillColor(kBlue);
  Eff_TrackThLowpt->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #theta, pt<250 MeV");

  TGraphAsymmErrors *Eff_TrackThHighpt = new TGraphAsymmErrors();
  Eff_TrackThHighpt  -> Divide(h_ThpiHighpt ,h_ThpiTruthHighpt ,"cl=0.683 b(1,1) mode");
  h_th->Draw();
  Eff_TrackThHighpt->SetFillColor(kBlue);
  Eff_TrackThHighpt->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #theta, pt>=250 MeV");

  TGraphAsymmErrors *Eff_TrackPhLowpt = new TGraphAsymmErrors();
  Eff_TrackPhLowpt  -> Divide(h_PhpiLowpt ,h_PhpiTruthLowpt ,"cl=0.683 b(1,1) mode");
  h_ph->Draw();
  Eff_TrackPhLowpt->SetFillColor(kBlue);
  Eff_TrackPhLowpt->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #phi, pt<250 MeV");

  TGraphAsymmErrors *Eff_TrackPhHighpt = new TGraphAsymmErrors();
  Eff_TrackPhHighpt  -> Divide(h_PhpiHighpt ,h_PhpiTruthHighpt ,"cl=0.683 b(1,1) mode");
  h_ph->Draw();
  Eff_TrackPhHighpt->SetFillColor(kBlue);
  Eff_TrackPhHighpt->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #phi, pt>=250 MeV");

  TFile* output = new TFile("EfficiencyValidationTracks.root", "recreate");
  TH1F *h_Eff_Track = (TH1F*)h_Ppi->Clone("hEffTrack");
  TH1F *h_Eff_TrackThLowpt  = (TH1F*)h_ThpiLowpt->Clone("hEffTrackThLowpt");
  TH1F *h_Eff_TrackThHighpt = (TH1F*)h_ThpiHighpt->Clone("hEffTrackThHighpt");
  TH1F *h_Eff_TrackPhLowpt  = (TH1F*)h_PhpiLowpt->Clone("hEffTrackPhLowpt");
  TH1F *h_Eff_TrackPhHighpt = (TH1F*)h_PhpiHighpt->Clone("hEffTrackPhHighpt");

  h_Eff_Track ->GetYaxis()->SetTitle("Track Reconstruction Efficiency");
  h_Eff_Track->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. A Generic BBbar sample is used."));
  h_Eff_Track->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency. Steep rise in efficiency up to 90 percent at 0.5 GeV. pT<0.5 GeV more sensitive to tracking algorithm changes."));

  h_Eff_TrackThLowpt ->GetYaxis()->SetTitle("Track Reconstruction Efficiency");
  h_Eff_TrackThLowpt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of theta_lab. A Generic BBbar sample is used, thus the sample is weighted to low momentum. Track cut pT<250 MeV"));
  h_Eff_TrackThLowpt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume."));

  h_Eff_TrackThHighpt ->GetYaxis()->SetTitle("Track Reconstruction Efficiency");
  h_Eff_TrackThHighpt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of theta_lab. A Generic BBbar sample is used, thus the sample is weighted to low momentum. Track cut pT>=250 MeV"));
  h_Eff_TrackThHighpt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume."));

  h_Eff_TrackPhLowpt ->GetYaxis()->SetTitle("Track Reconstruction Efficiency");
  h_Eff_TrackPhLowpt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of phi_lab. A Generic BBbar sample is used, thus the sample is weighted to low momentum. Track cut pT<250 MeV"));
  h_Eff_TrackPhLowpt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Periodic dip at #pi/2 modulo."));

  h_Eff_TrackPhHighpt ->GetYaxis()->SetTitle("Track Reconstruction Efficiency");
  h_Eff_TrackPhHighpt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of phi_lab. A Generic BBbar sample is used, thus the sample is weighted to low momentum. Track cut pT>=250 MeV"));
  h_Eff_TrackPhHighpt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Periodic dip at #pi/2 modulo."));

  h_Eff_Track ->Divide(h_Ppi,h_PpiTruth,1,1);
  h_Eff_TrackThLowpt  ->Divide(h_ThpiLowpt,h_ThpiTruthLowpt,1,1);
  h_Eff_TrackThHighpt ->Divide(h_ThpiHighpt,h_ThpiTruthHighpt,1,1);
  h_Eff_TrackPhLowpt  ->Divide(h_PhpiLowpt,h_PhpiTruthLowpt,1,1);
  h_Eff_TrackPhHighpt ->Divide(h_PhpiHighpt,h_PhpiTruthHighpt,1,1);
  output->Write();
  output->Close();
  maincanvas->cd()->SetGridy(0);
  maincanvas->cd()->SetGridx(0);

  ///////////////////// Photons
  h_PgammaTruth->SetMaximum(h_PgammaTruth->GetMaximum()*1.2);
  h_PgammaTruth->SetMinimum(0.);
  h_PgammaTruth->SetLineColor(kBlue);
  h_PgammaTruth->Draw();
  h_Pgamma->SetLineColor(kRed);
  h_Pgamma->SetLineStyle(2);
  h_Pgamma->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon p_{T} truth and reco");

  h_ThgammaTruthLowE->SetMaximum(h_ThgammaTruthLowE->GetMaximum()*1.2);
  h_ThgammaTruthLowE->SetMinimum(0);
  h_ThgammaTruthLowE->SetLineColor(kBlue);
  h_ThgammaTruthLowE->Draw();
  h_ThgammaLowE->SetLineColor(kRed);
  h_ThgammaLowE->SetLineStyle(2);
  h_ThgammaLowE->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon #theta_{lab} truth and reco, E<500 MeV");

  h_ThgammaTruthHighE->SetMaximum(h_ThgammaTruthHighE->GetMaximum()*1.2);
  h_ThgammaTruthHighE->SetMinimum(0);
  h_ThgammaTruthHighE->SetLineColor(kBlue);
  h_ThgammaTruthHighE->Draw();
  h_ThgammaHighE->SetLineColor(kRed);
  h_ThgammaHighE->SetLineStyle(2);
  h_ThgammaHighE->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon #theta_{lab} truth and reco, E>=500 MeV");

  h_PhgammaTruthLowE->SetMaximum(h_PhgammaTruthLowE->GetMaximum()*1.2);
  h_PhgammaTruthLowE->SetMinimum(0);
  h_PhgammaTruthLowE->SetLineColor(kBlue);
  h_PhgammaLowE->SetLineColor(kRed);
  h_PhgammaLowE->SetLineStyle(2);
  h_PhgammaTruthLowE->Draw();
  h_PhgammaLowE->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon #phi_{lab} truth and reco, E<500 MeV");

  h_PhgammaTruthHighE->SetMaximum(h_PhgammaTruthHighE->GetMaximum()*1.2);
  h_PhgammaTruthHighE->SetMinimum(0);
  h_PhgammaTruthHighE->SetLineColor(kBlue);
  h_PhgammaHighE->SetLineColor(kRed);
  h_PhgammaHighE->SetLineStyle(2);
  h_PhgammaTruthHighE->Draw();
  h_PhgammaHighE->Draw("same");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon #phi_{lab} truth and reco, E>=500 MeV");

  TH1F * h_pg   = new TH1F("pdummy"  ,"Eff Photon;E GeV; Photon Efficiency",10,plow,phigh);
  TH1F * h_thg  = new TH1F("thdummy" ,"Eff Photon;#theta ; Photon Efficiency",10,thlow,thhigh);
  TH1F * h_phg  = new TH1F("phdummy" ,"Eff Photon;#phi ; Photon Efficiency",10,phlow,phhigh);

  TGraphAsymmErrors *Eff_Photon = new TGraphAsymmErrors();
  Eff_Photon  -> Divide(h_Pgamma ,h_PgammaTruth ,"cl=0.683 b(1,1) mode");
  h_pg->Draw();
  Eff_Photon->SetFillColor(kBlue);
  Eff_Photon->SetMarkerColor(kBlue);
  Eff_Photon->SetMarkerStyle(21);
  Eff_Photon->Draw("2p");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency E");

  TGraphAsymmErrors *Eff_PhotonThLowE = new TGraphAsymmErrors();
  Eff_PhotonThLowE  -> Divide(h_ThgammaLowE ,h_ThgammaTruthLowE ,"cl=0.683 b(1,1) mode");
  h_thg->Draw();
  Eff_PhotonThLowE->SetFillColor(kBlue);
  Eff_PhotonThLowE->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency #theta, E<500 MeV");

  TGraphAsymmErrors *Eff_PhotonThHighE = new TGraphAsymmErrors();
  Eff_PhotonThHighE  -> Divide(h_ThgammaHighE ,h_ThgammaTruthHighE ,"cl=0.683 b(1,1) mode");
  h_thg->Draw();
  Eff_PhotonThHighE->SetFillColor(kBlue);
  Eff_PhotonThHighE->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency #theta, E>=500 MeV");

  TGraphAsymmErrors *Eff_PhotonPhLowE = new TGraphAsymmErrors();
  Eff_PhotonPhLowE  -> Divide(h_PhgammaLowE ,h_PhgammaTruthLowE ,"cl=0.683 b(1,1) mode");
  h_phg->Draw();
  Eff_PhotonPhLowE->SetFillColor(kBlue);
  Eff_PhotonPhLowE->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency #phi, E<500 MeV");

  TGraphAsymmErrors *Eff_PhotonPhHighE = new TGraphAsymmErrors();
  Eff_PhotonPhHighE  -> Divide(h_PhgammaHighE ,h_PhgammaTruthHighE ,"cl=0.683 b(1,1) mode");
  h_phg->Draw();
  Eff_PhotonPhHighE->SetFillColor(kBlue);
  Eff_PhotonPhHighE->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  maincanvas->Print("efficiency.pdf)","Title:Photon efficiency #phi, E>=500 MeV");

  TFile* outputP = new TFile("EfficiencyValidationPhotons.root", "recreate");
  TH1F *h_Eff_Photon = (TH1F*)h_Pgamma->Clone("hEffPhoton");
  TH1F *h_Eff_PhotonThLowE = (TH1F*)h_ThgammaLowE->Clone("hEffPhotonThLowE");
  TH1F *h_Eff_PhotonThHighE = (TH1F*)h_ThgammaHighE->Clone("hEffPhotonThHighE");
  TH1F *h_Eff_PhotonPhLowE = (TH1F*)h_PhgammaLowE->Clone("hEffPhotonPhLowE");
  TH1F *h_Eff_PhotonPhHighE = (TH1F*)h_PhgammaHighE->Clone("hEffPhotonPhHighE");

  h_Eff_Photon ->GetYaxis()->SetTitle("Photon Reconstruction Efficiency");
  h_Eff_Photon->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction efficiency of truth-matched photons in bins of energy. Input: Generic BBbar."));
  h_Eff_Photon->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency, particularly at low E. Steep rise to 80 percent at 1 GeV."));

  h_Eff_PhotonThLowE ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonThLowE->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction efficiency of truth-matched photons in bins of theta_lab. Input: Generic BBbar, thus the sample is weighted to low energy. Photon cut E<500 MeV."));
  h_Eff_PhotonThLowE->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Dips at 0.6 and 2.3."));

  h_Eff_PhotonThHighE ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonThHighE->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction efficiency of truth-matched photons in bins of theta_lab. Input: Generic BBbar, thus the sample is weighted to low energy. Photon cut E>=500 MeV."));
  h_Eff_PhotonThHighE->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Dips at 0.6 and 2.3."));

  h_Eff_PhotonPhLowE ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonPhLowE->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction efficiency of truth-matched photons in bins of phi_lab. Input: Generic BBbar, thus the sample is weighted to low energy. Photon cut E<500 MeV."));
  h_Eff_PhotonPhLowE->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Flat across the spectrum."));

  h_Eff_PhotonPhHighE ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonPhHighE->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction efficiency of truth-matched photons in bins of phi_lab. Input: Generic BBbar, thus the sample is weighted to low energy. Photon cut E>=500 MeV."));
  h_Eff_PhotonPhHighE->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Flat across the spectrum."));

  h_Eff_Photon ->Divide(h_Pgamma,h_PgammaTruth,1,1);
  h_Eff_PhotonThLowE  ->Divide(h_ThgammaLowE ,h_ThgammaTruthLowE ,1,1);
  h_Eff_PhotonThHighE ->Divide(h_ThgammaHighE,h_ThgammaTruthHighE,1,1);
  h_Eff_PhotonPhLowE  ->Divide(h_PhgammaLowE ,h_PhgammaTruthLowE ,1,1);
  h_Eff_PhotonPhHighE ->Divide(h_PhgammaHighE,h_PhgammaTruthHighE,1,1);
  outputP->Write();
  outputP->Close();
   
}
void test2_Validation_Efficiency_Truth(TString filename){

  
  /*  Take the pituple prepared by the NtupleMaker */
  TChain * truthTree = new TChain("truthpituple");
  truthTree->AddFile(filename);

  float fpi_P4[4];  
  truthTree->SetBranchAddress("pi_P4",      &fpi_P4);  

  for(Int_t iloop=0;iloop<truthTree->GetEntries();iloop++) {
    truthTree->GetEntry(iloop);
    TLorentzVector lv_pi(fpi_P4);  

    h_PpiTruth->Fill(lv_pi.Pt());
    if(lv_pi.Theta()>2.62 || lv_pi.Theta()<0.3) continue;

    if(lv_pi.Pt()<0.25){
      h_ThpiTruthLowpt->Fill(lv_pi.Theta());
      h_PhpiTruthLowpt->Fill(lv_pi.Phi());
    }

    if(lv_pi.Pt()>=0.25){
      h_ThpiTruthHighpt->Fill(lv_pi.Theta());
      h_PhpiTruthHighpt->Fill(lv_pi.Phi());
    }

  }

}

void test2_Validation_Efficiency_Reco(TString filename){

  TChain * recoTree = new TChain("pituple");
  recoTree->AddFile(filename);

  float fpi_TruthP4[4];  
  float fpi_P4[4];  
  float fpi_dz;  
  int ipi_mcID;  
  int iCand;  
  recoTree->SetBranchAddress("pi_mcID", &ipi_mcID);  
  recoTree->SetBranchAddress("pi_TruthP4", &fpi_TruthP4);  
  recoTree->SetBranchAddress("pi_P4",      &fpi_P4);  
  recoTree->SetBranchAddress("pi_dz",      &fpi_dz);  
  recoTree->SetBranchAddress("iCand",      &iCand);  
  
  // Run a check on track duplicates
  double cache_p[30];
  double epsilon_p=0.001;
  int count_pi=0;
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_pi_truth(fpi_TruthP4);  
    TLorentzVector lv_pi(fpi_P4);  
    
    if(iCand==0){
      for(int i=0;i<30;i++)cache_p[i]=0;
      count_pi=0;
    }
    if(lv_pi_truth.Theta()>2.62 || lv_pi_truth.Theta()<0.3) continue;

    int pdgid=abs(ipi_mcID);
    if(pdgid==pid[pion]){
      cache_p[count_pi]=lv_pi_truth.Pt();
      h_Ppi->Fill(lv_pi_truth.Pt());
      
      if(lv_pi_truth.Pt()<0.25){
	h_ThpiLowpt->Fill(lv_pi_truth.Theta());
	h_PhpiLowpt->Fill(lv_pi_truth.Phi());
      }
      
      if(lv_pi_truth.Pt()>=0.25){
	h_ThpiHighpt->Fill(lv_pi_truth.Theta());
	h_PhpiHighpt->Fill(lv_pi_truth.Phi());
      }

      count_pi++;
    }
  }
  
 
}


void test2_Validation_Efficiency_Photon_Truth(TString filename, int region){

  TPaveText *belleName = new TPaveText(0.6,0.8,0.9,0.9,"BRNDC");
  belleName->SetFillColor(0);
  belleName->SetTextColor(kGray);
  belleName->SetTextAlign(12);
  belleName->SetBorderSize(0);
  belleName->AddText("Belle II Validation");
  
  /*  Take the pituple prepared by the NtupleMaker */
  TChain * truthTree = new TChain("truthgammatuple");
  truthTree->AddFile(filename);

  float fgamma_P4[4];  
  truthTree->SetBranchAddress("gamma_P4",      &fgamma_P4);  
  
  for(Int_t iloop=0;iloop<truthTree->GetEntries();iloop++) {
    truthTree->GetEntry(iloop);
    TLorentzVector lv_gamma(fgamma_P4);  
    h_PgammaTruth->Fill(lv_gamma.E());

    if(lv_gamma.E()<0.02) continue;
    if(lv_gamma.Theta()>2.71 || lv_gamma.Theta()<0.21) continue;
    if((lv_gamma.Theta()>2.23 || lv_gamma.Theta()<0.58) && region==1) continue; //baRrel
    if((lv_gamma.Theta()>0.58 || lv_gamma.Theta()<0.21) && region==2) continue; //forward
    if((lv_gamma.Theta()>2.71 || lv_gamma.Theta()<2.23) && region==3) continue; //backward

    if(lv_gamma.E()<0.5){
      h_ThgammaTruthLowE->Fill(lv_gamma.Theta());
      h_PhgammaTruthLowE->Fill(lv_gamma.Phi());
    }
    if(lv_gamma.E()>=0.5){
      h_ThgammaTruthHighE->Fill(lv_gamma.Theta());
      h_PhgammaTruthHighE->Fill(lv_gamma.Phi());
    }


  }

}

void test2_Validation_Efficiency_Photon_Reco(TString filename, int region){

  TChain * recoTree = new TChain("gammatuple");
  recoTree->AddFile(filename);

  float fgamma_TruthP4[4];  
  float fgamma_P4[4];  
  float fgamma_TruthP;  
  float fgamma_P;  
  float fgamma_dz;  
  int igamma_mcID;  
  int iCand;  
  recoTree->SetBranchAddress("gamma_mcID", &igamma_mcID);  
  recoTree->SetBranchAddress("gamma_TruthP4", &fgamma_TruthP4);  
  recoTree->SetBranchAddress("gamma_P4",      &fgamma_P4);  
  recoTree->SetBranchAddress("gamma_TruthP",  &fgamma_TruthP);  
  recoTree->SetBranchAddress("gamma_P",       &fgamma_P);  
  recoTree->SetBranchAddress("gamma_dz",      &fgamma_dz);  
  recoTree->SetBranchAddress("iCand",         &iCand);  
  
  // Run a check on track duplicates
  double cache_p;
  double epsilon_p=0.001;
  int count_gamma=0;
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_gamma_truth(fgamma_TruthP4);  
    TLorentzVector lv_gamma(fgamma_P4);  
    
    if(iCand==0){
      cache_p=0;
      count_gamma=0;
    }

    if(lv_gamma_truth.Theta()>2.71 || lv_gamma_truth.Theta()<0.21) continue;
    if((lv_gamma_truth.Theta()>2.23 || lv_gamma_truth.Theta()<0.58) && region==1) continue; //baRrel
    if((lv_gamma_truth.Theta()>0.58 || lv_gamma_truth.Theta()<0.21) && region==2) continue; //forward
    if((lv_gamma_truth.Theta()>2.71 || lv_gamma_truth.Theta()<2.23) && region==3) continue; //backward

    int pdgid=abs(igamma_mcID);
    if(pdgid==22){
      if(iCand>0 && fabs(cache_p-lv_gamma_truth.Rho())<epsilon_p) continue;
      cache_p=lv_gamma_truth.Rho();
      h_Pgamma->Fill(fgamma_TruthP);
      
      if(lv_gamma_truth.E()<0.5){
	h_ThgammaLowE->Fill(lv_gamma_truth.Theta());
	h_PhgammaLowE->Fill(lv_gamma_truth.Phi());
      }
      if(lv_gamma_truth.E()>=0.5){
	h_ThgammaHighE->Fill(lv_gamma_truth.Theta());
	h_PhgammaHighE->Fill(lv_gamma_truth.Phi());
      }

      count_gamma++;
      iCand++;
    }
  }
  
 
}

