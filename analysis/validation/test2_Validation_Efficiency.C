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

const int nthbins=16;
const float thlow=0;
const float thhigh=3.2;
const float phlow=-3.2;
const float phhigh=3.2;

const int npbins=13;
const float plow=0.;
const float phigh=2.6;

const int nptbins=26;
const float ptlow=0.;
const float pthigh=2.6;

//Lab momentum
TH1F * h_PpiTruth  = new TH1F("P_pi_Truth"       ,";p_{T}(#pi);N"      ,nptbins,ptlow,pthigh);
//Lab cos(theta)
TH1F * h_ThpiTruth = new TH1F("Theta_pi_Truth",";#theta(#pi) ;N" ,nthbins,thlow,thhigh);	
//Lab phi
TH1F * h_PhpiTruth = new TH1F("Phi_pi_Truth",";#phi(#pi) ;N" ,nthbins,phlow,phhigh);	

TH1F * h_Ppi       = new TH1F("P_pi_"    ,";p_{T};N"      ,nptbins,ptlow,pthigh);
TH1F * h_Thpi      = new TH1F("Theta_pi_",";#theta(#pi) ;N" ,nthbins,thlow,thhigh);	
TH1F * h_Phpi      = new TH1F("Phi_pi_"  ,";#phi(#pi) ;N" ,nthbins,phlow,phhigh);	

//Photons
TH1F * h_PgammaTruth  = new TH1F("P_gamma_Truth"       ,";p_{T}(#gamma);N"      ,npbins,plow,phigh);
TH1F * h_ThgammaTruth = new TH1F("Theta_gamma_Truth",";#theta(#gamma) ;N" ,nthbins,thlow,thhigh);	
TH1F * h_PhgammaTruth = new TH1F("Phi_gamma_Truth",";#phi(#gamma) ;N" ,nthbins,phlow,phhigh);	

TH1F * h_Pgamma       = new TH1F("P_gamma_"    ,";p_{T};N"      ,npbins,plow,phigh);
TH1F * h_Thgamma      = new TH1F("Theta_gamma_",";#theta(#gamma) ;N" ,nthbins,thlow,thhigh);	
TH1F * h_Phgamma      = new TH1F("Phi_gamma_"  ,";#phi(#gamma) ;N" ,nthbins,phlow,phhigh);	

void test2_Validation_Efficiency_Truth(TString, int);
void test2_Validation_Efficiency_Reco();
void test2_Validation_Efficiency_Photon_Truth();
void test2_Validation_Efficiency_Photon_Reco();

void test2_Validation_Efficiency(){
  
  TString testfile ("../GenericB.ntup.root");

  TPaveText *belleName = new TPaveText(0.6,0.9,0.9,0.95,"BRNDC");
  belleName->SetFillColor(0);
  belleName->SetTextColor(kGray);
  belleName->SetTextAlign(12);
  belleName->SetBorderSize(0);
  belleName->AddText("Belle II Validation");

  test2_Validation_Efficiency_Truth(testfile,0);
  test2_Validation_Efficiency_Reco();
  test2_Validation_Efficiency_Photon_Truth();
  test2_Validation_Efficiency_Photon_Reco();

  TCanvas *maincanvas = new TCanvas ("maincanvas","maincanvas");

  ///////////////////// Tracks
  h_PpiTruth->SetMaximum(h_PpiTruth->GetMaximum()*1.2);
  h_PpiTruth->SetMinimum(0.);
  h_PpiTruth->SetLineColor(kBlue);
  h_PpiTruth->Draw();
  h_Ppi->Draw("same");belleName->Draw();
  maincanvas->Print("efficiency.pdf(","Title:Track p_{T} truth and reco");

  h_ThpiTruth->SetMaximum(h_ThpiTruth->GetMaximum()*1.2);
  h_ThpiTruth->SetMinimum(0);
  h_ThpiTruth->SetLineColor(kBlue);
  h_ThpiTruth->Draw();
  h_Thpi->Draw("same");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Track #theta_{lab} truth and reco");

  h_PhpiTruth->SetMaximum(h_PhpiTruth->GetMaximum()*1.2);
  h_PhpiTruth->SetMinimum(0);
  h_PhpiTruth->SetLineColor(kBlue);
  h_PhpiTruth->Draw();
  h_Phpi->Draw("same");belleName->Draw();  
  maincanvas->Print("efficiency.pdf","Title:Track #phi_{lab} truth and reco");

  TH1F * h_p  = new TH1F("pdummy","Eff Track;p_{T} GeV; Track Efficiency",10,ptlow,pthigh);
  TH1F * h_th  = new TH1F("thdummy","Eff Track;#theta ; Track Efficiency",10,thlow,thhigh);
  TH1F * h_ph  = new TH1F("phdummy","Eff Track;#phi ; Track Efficiency",10,phlow,phhigh);

  
  TGraphAsymmErrors *Eff_Track = new TGraphAsymmErrors();
  Eff_Track  -> Divide(h_Ppi ,h_PpiTruth ,"cl=0.683 b(1,1) mode");
  h_p->Draw();
  Eff_Track->SetFillColor(kBlue);
  Eff_Track->Draw("2");belleName->Draw();
  cout<<h_Ppi->Integral()/h_PpiTruth->Integral()<<endl;
  maincanvas->Print("efficiency.pdf","Title:Track efficiency p_{T}");

  TGraphAsymmErrors *Eff_TrackTh = new TGraphAsymmErrors();
  Eff_TrackTh  -> Divide(h_Thpi ,h_ThpiTruth ,"cl=0.683 b(1,1) mode");
  h_th->Draw();
  Eff_TrackTh->SetFillColor(kBlue);
  Eff_TrackTh->Draw("2");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #theta");

  TGraphAsymmErrors *Eff_TrackPh = new TGraphAsymmErrors();
  Eff_TrackPh  -> Divide(h_Phpi ,h_PhpiTruth ,"cl=0.683 b(1,1) mode");
  h_ph->Draw();
  Eff_TrackPh->SetFillColor(kBlue);
  Eff_TrackPh->Draw("2");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Track efficiency #phi");

  TFile* output = new TFile("EfficiencyValidationTracks.root", "recreate");
  TH1F *h_Eff_Track = (TH1F*)h_Ppi->Clone("hEffTrack");
  TH1F *h_Eff_TrackTh = (TH1F*)h_Thpi->Clone("hEffTrackTh");
  TH1F *h_Eff_TrackPh = (TH1F*)h_Phpi->Clone("hEffTrackPh");
  h_Eff_Track ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_TrackTh ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_TrackPh ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_Track ->Divide(h_Ppi,h_PpiTruth,1,1);
  h_Eff_TrackTh ->Divide(h_Thpi,h_ThpiTruth,1,1);
  h_Eff_TrackPh ->Divide(h_Phpi,h_PhpiTruth,1,1);
  output->Write();
  output->Close();
  ///////////////////// Photons
  h_PgammaTruth->SetMaximum(h_PgammaTruth->GetMaximum()*1.2);
  h_PgammaTruth->SetMinimum(0.);
  h_PgammaTruth->SetLineColor(kBlue);
  h_PgammaTruth->Draw();
  h_Pgamma->Draw("same");belleName->Draw();
  maincanvas->Print("efficiency.pdf(","Title:Photon p_{T} truth and reco");

  h_ThgammaTruth->SetMaximum(h_ThgammaTruth->GetMaximum()*1.2);
  h_ThgammaTruth->SetMinimum(0);
  h_ThgammaTruth->SetLineColor(kBlue);
  h_ThgammaTruth->Draw();
  h_Thgamma->Draw("same");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Photon #theta_{lab} truth and reco");

  h_PhgammaTruth->SetMaximum(h_PhgammaTruth->GetMaximum()*1.2);
  h_PhgammaTruth->SetMinimum(0);
  h_PhgammaTruth->SetLineColor(kBlue);
  h_PhgammaTruth->Draw();
  h_Phgamma->Draw("same");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Photon #phi_{lab} truth and reco");

  TH1F * h_pg  = new TH1F("pdummy","Eff Photon;E GeV; Photon Efficiency",10,plow,phigh);
  TH1F * h_thg  = new TH1F("thdummy","Eff Photon;#theta ; Photon Efficiency",10,thlow,thhigh);
  TH1F * h_phg  = new TH1F("phdummy","Eff Photon;#phi ; Photon Efficiency",10,phlow,phhigh);

  TGraphAsymmErrors *Eff_Photon = new TGraphAsymmErrors();
  Eff_Photon  -> Divide(h_Pgamma ,h_PgammaTruth ,"cl=0.683 b(1,1) mode");
  h_pg->Draw();
  Eff_Photon->SetFillColor(kBlue);
  Eff_Photon->Draw("2");belleName->Draw();
  cout<<h_Pgamma->Integral()/h_PgammaTruth->Integral()<<endl;
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency E");

  TGraphAsymmErrors *Eff_PhotonTh = new TGraphAsymmErrors();
  Eff_PhotonTh  -> Divide(h_Thgamma ,h_ThgammaTruth ,"cl=0.683 b(1,1) mode");
  h_thg->Draw();
  Eff_PhotonTh->SetFillColor(kBlue);
  Eff_PhotonTh->Draw("2");belleName->Draw();
  maincanvas->Print("efficiency.pdf","Title:Photon efficiency #theta");

  TGraphAsymmErrors *Eff_PhotonPh = new TGraphAsymmErrors();
  Eff_PhotonPh  -> Divide(h_Phgamma ,h_PhgammaTruth ,"cl=0.683 b(1,1) mode");
  h_phg->Draw();
  Eff_PhotonPh->SetFillColor(kBlue);
  Eff_PhotonPh->Draw("2");belleName->Draw();
  maincanvas->Print("efficiency.pdf)","Title:Photon efficiency #phi");

  TFile* outputP = new TFile("EfficiencyValidationPhotons.root", "recreate");
  TH1F *h_Eff_Photon = (TH1F*)h_Pgamma->Clone("hEffPhoton");
  TH1F *h_Eff_PhotonTh = (TH1F*)h_Thgamma->Clone("hEffPhotonTh");
  TH1F *h_Eff_PhotonPh = (TH1F*)h_Phgamma->Clone("hEffPhotonPh");
  h_Eff_Photon ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonTh ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_PhotonPh ->GetYaxis()->SetTitle("Efficiency");
  h_Eff_Photon ->Divide(h_Pgamma,h_PgammaTruth,1,1);
  h_Eff_PhotonTh ->Divide(h_Thgamma,h_ThgammaTruth,1,1);
  h_Eff_PhotonPh ->Divide(h_Phgamma,h_PhgammaTruth,1,1);
  outputP->Write();
  outputP->Close();
   
}
void test2_Validation_Efficiency_Truth(TString filename, int testindex){

  
  /*  Take the pituple prepared by the NtupleMaker */
  TChain * truthTree = new TChain("truthpituple");
  truthTree->AddFile(filename);

  float fpi_P4[4];  
  truthTree->SetBranchAddress("pi_P4",      &fpi_P4);  
  
  for(Int_t iloop=0;iloop<truthTree->GetEntries();iloop++) {
    truthTree->GetEntry(iloop);
    TLorentzVector lv_pi(fpi_P4);  
    h_PpiTruth->Fill(lv_pi.Pt());
    h_ThpiTruth->Fill(lv_pi.Theta());
    h_PhpiTruth->Fill(lv_pi.Phi());
  }

}

void test2_Validation_Efficiency_Reco(){

  TChain * recoTree = new TChain("pituple");
  recoTree->AddFile("../GenericB.ntup.root");

  float fpi_TruthP4[4];  
  float fpi_P4[4];  
  float fpi_dz;  
  int ipi_TruthID;  
  int iCand;  
  recoTree->SetBranchAddress("pi_TruthID", &ipi_TruthID);  
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

    int pdgid=abs(ipi_TruthID);
    if(pdgid==pid[pion]){
      //if(count_pi>0)
      //      if(iCand>0 && cache_p[iCand-1]-lv_pi_truth.Rho()<epsilon_p) continue;
      cache_p[count_pi]=lv_pi_truth.Pt();
      h_Ppi->Fill(lv_pi_truth.Pt());
      h_Thpi->Fill(lv_pi_truth.Theta());
      h_Phpi->Fill(lv_pi_truth.Phi());
      count_pi++;
    }
  }
  
 
}


void test2_Validation_Efficiency_Photon_Truth(){

  TPaveText *belleName = new TPaveText(0.6,0.8,0.9,0.9,"BRNDC");
  belleName->SetFillColor(0);
  belleName->SetTextColor(kGray);
  belleName->SetTextAlign(12);
  belleName->SetBorderSize(0);
  belleName->AddText("Belle II Validation");
  
  /*  Take the pituple prepared by the NtupleMaker */
  TChain * truthTree = new TChain("truthgammatuple");
  truthTree->AddFile("../GenericB.ntup.root");

  float fgamma_P4[4];  
  truthTree->SetBranchAddress("gamma_P4",      &fgamma_P4);  
  
  for(Int_t iloop=0;iloop<truthTree->GetEntries();iloop++) {
    truthTree->GetEntry(iloop);
    TLorentzVector lv_gamma(fgamma_P4);  
    h_PgammaTruth->Fill(lv_gamma.E());
    h_ThgammaTruth->Fill(lv_gamma.Theta());
    h_PhgammaTruth->Fill(lv_gamma.Phi());
  }

}

void test2_Validation_Efficiency_Photon_Reco(){

  TChain * recoTree = new TChain("gammatuple");
  recoTree->AddFile("../GenericB.ntup.root");

  float fgamma_TruthP4[4];  
  float fgamma_P4[4];  
  float fgamma_TruthP;  
  float fgamma_P;  
  float fgamma_dz;  
  int igamma_TruthID;  
  int iCand;  
  recoTree->SetBranchAddress("gamma_TruthID", &igamma_TruthID);  
  recoTree->SetBranchAddress("gamma_TruthP4", &fgamma_TruthP4);  
  recoTree->SetBranchAddress("gamma_P4",      &fgamma_P4);  
  recoTree->SetBranchAddress("gamma_TruthP",  &fgamma_TruthP);  
  recoTree->SetBranchAddress("gamma_P",       &fgamma_P);  
  recoTree->SetBranchAddress("gamma_dz",      &fgamma_dz);  
  recoTree->SetBranchAddress("iCand",         &iCand);  
  
  // Run a check on track duplicates
  double cache_p[30];
  double epsilon_p=0.001;
  int count_gamma=0;
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_gamma_truth(fgamma_TruthP4);  
    TLorentzVector lv_gamma(fgamma_P4);  
    
    if(iCand==0){
      for(int i=0;i<30;i++)cache_p[i]=0;
      count_gamma=0;
    }

    int pdgid=abs(igamma_TruthID);
    if(pdgid==22){
      //if(count_gamma>0)
      //      if(iCand>0 && cache_p[iCand-1]-lv_gamma_truth.Rho()<epsilon_p) continue;
      cache_p[count_gamma]=lv_gamma_truth.Pt();
      //h_Pgamma->Fill(lv_gamma_truth.E());
      h_Pgamma->Fill(fgamma_TruthP);
      h_Thgamma->Fill(lv_gamma_truth.Theta());
      h_Phgamma->Fill(lv_gamma_truth.Phi());
      count_gamma++;
    }
  }
  
 
}

