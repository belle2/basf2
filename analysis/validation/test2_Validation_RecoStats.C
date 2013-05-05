////////////////////////////////////////////////////////////
// Validation_RecoStats.cc
// Check the detector sim and reco stats
//
// Phillip Urquijo
// May 3, 2013
//
gSystem->Load("libRooFit.so");
using namespace RooFit ;
void test2_Validation_RecoStats(){
  
  TChain * recoTree = new TChain("eventTuple");
  recoTree->AddFile("../GenericB.ntup.root");

  const char *RecoVars[] = { 
    /* Analysis Level */ "MCParticles", "Particles","PIDLikelihoods", 

    /* Reco level*/
    /* ARICH */"ARICHAeroHits", "ARICHLikelihoods",
    /* CDC */"CDCHits",
    /* DedxLikelihoods*/ "DedxLikelihoods",
    /* ECL */ "ECLDsps", "ECLHitAssignments", "ECLHits", "ECLGammas", "ECLPi0s", "ECLShowers", "ECLTrigs",
    /* EKLM */ "EKLMHit2ds", "EKLMK0Ls", "EKLMStepHits",
    /* PXD */ "PXDClusters",
    /* SVD */ "SVDClusters",
    /* TOP */ "TOPBarHits", "TOPLikelihoods",
    /* Tracks */ "Tracks", "TrackFitResults", "GFTracks",

    /* SIM level */
    /* ARICH */ "ARICHDigits", "ARICHSimHits",
    /* CDC */ "CDCSimHits",
    /* ECL */ "ECLDigits", "ECLSimHits",
    /* EKLM */ "EKLMDigits", "EKLMSimHits",
    /* PXD */ "PXDDigits", "PXDSimHits","PXDTrueHits",
    /* SVD */ "SVDDigits", "SVDSimHits","SVDTrueHits",
    /* TOP */ "TOPDigits", "TOPSimHits","TOPSimPhotons",
  };

  const int RecoVarRanges[] = { 
    /* Analysis Level*/ 3000, 500, 50, 

    /* Reco Level */
    /* ARICH*/ 20, 20,
    /* CDC*/ 2500,
    /* DedxLikelihoods*/ 20,
    /* ECL */ 1000, 500, 4000, 50, 100, 100, 5,
    /* EKLM */ 100, 10, 1000,
    /* PXD */ 100,
    /* SVD */ 400,
    /* TOP */ 1000, 20,
    /* Tracks */ 50, 50, 50,

    /* Sim Level */
    /* ARICH */ 500,500,
    /* CDC*/  10500,
    /* ECL */ 1500, 20000,
    /* EKLM */ 500, 500,
    /* PXD */ 500,1500,100,
    /* SVD */ 20000,20000,200,
    /* TOP */ 1000,800,800
  };

  
  TH1F *h_Reco[50]; 

  Float_t RecoMeans[40];
  Float_t RecoRMS[40];

  TCanvas *tc = new TCanvas ("tcReco","tcReco",1000,800);
  tc->Divide(6,4);
  
  for(int i=0;i<24;i++){
    tc->cd(i+1);
    int nbins = RecoVarRanges[i];
    if(nbins>50)nbins/=(int)(RecoVarRanges[i]/50);

    h_Reco[i]= new TH1F(Form("h%s",string(RecoVars[i]).c_str()),Form(";%s;Events",string(RecoVars[i]).c_str()),nbins,-0.5,RecoVarRanges[i]-0.5);
    recoTree->Draw(Form("n%s>>h%s",string(RecoVars[i]).c_str(),string(RecoVars[i]).c_str()),"","");
    h_Reco[i]->SetLineColor(kBlue);
    h_Reco[i]->Draw("hist");
    cout<<RecoVars[i]<<" \t\t"<<h_Reco[i]->GetMean();
    cout<<" \t\t"<<h_Reco[i]->GetRMS()<<endl;
    RecoMeans[i]=h_Reco[i]->GetMean();
    RecoRMS[i]=h_Reco[i]->GetRMS();
  }

  TCanvas *tcs = new TCanvas ("tcSim","tcSim",1000,800);
  tcs->Divide(4,4);
  
  for(int i=24;i<40;i++){
    tcs->cd(i+1-24);
    int nbins = RecoVarRanges[i];
    if(nbins>50)nbins/=(int)(RecoVarRanges[i]/50);

    h_Reco[i]= new TH1F(Form("h%s",string(RecoVars[i]).c_str()),Form(";%s;Events",string(RecoVars[i]).c_str()),nbins,-0.5,RecoVarRanges[i]-0.5);
    recoTree->Draw(Form("n%s>>h%s",string(RecoVars[i]).c_str(),string(RecoVars[i]).c_str()),"","");
    h_Reco[i]->SetLineColor(kBlue);
    h_Reco[i]->Draw("hist");
    cout<<RecoVars[i]<<" \t\t"<<h_Reco[i]->GetMean();
    cout<<" \t\t"<<h_Reco[i]->GetRMS()<<endl;
    RecoMeans[i]=h_Reco[i]->GetMean();
    RecoRMS[i]=h_Reco[i]->GetRMS();
  }

  string namelist;
  for(int i=0;i<40;i++) namelist = namelist + Form("%s:",string(RecoVars[i]).c_str());
  cout<<namelist<<endl;
  TFile * output = TFile::Open("RecoStats.root", "recreate");
  TNtuple* trecostats = new TNtuple("Reco Stats in Nevents", "tree", Form("%s",string(namelist).c_str()));
  trecostats->Fill(RecoMeans);
  trecostats->Write();
  delete output;
}
