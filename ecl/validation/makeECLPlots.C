/* makePlots.C 
 * ROOT macro for ECL validation plots 
 * Author: Benjamin Oberhof 
 * 2015-06
*/

/*
<header>
<input>ECLBkgOutput.root, ECLClusterOutput.root, ECLMuonOutput.root, ECLPionOutput.root</input>
<output>ECLBkg.root, ECL2D.root, ECLMuon.root, ECLPion.root, ECLCluster.root</output>
<contact>ecl2ml@bpost.kek.jp</contact>
</header>
*/

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>
#include <TCut.h>

void ECL2D(TTree* bkg_tree);
void ECLBkg(TTree* bkg_tree);
void ECLCluster(TTree* cluster_tree);
void ECLMuon(TTree* muon_tree);
void ECLPion(TTree* pion_tree);

void makeECLPlots() 
{
  
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));

  TFile* bkg_input = TFile::Open("../ECLBkgOutput.root");
  TTree* bkg_tree = (TTree*) bkg_input->Get("m_tree");
  TFile* cluster_input = TFile::Open("../ECLClusterOutput.root");
  TTree* cluster_tree = (TTree*) cluster_input->Get("m_tree");
  TFile* muon_input = TFile::Open("../ECLMuonOutput.root");
  TTree* muon_tree = (TTree*) muon_input->Get("m_tree");
  TFile* pion_input = TFile::Open("../ECLPionOutput.root");
  TTree* pion_tree = (TTree*) pion_input->Get("m_tree");

  ECL2D(bkg_tree);
  ECLBkg(bkg_tree);
  ECLCluster(cluster_tree);
  ECLMuon(muon_tree);
  //  ECLPi0(pi0_tree);
  ECLPion(pion_tree);

}
/*
void ECLPi0(TTree* pi0_tree)
{

  TFile* output = TFile::Open("ECLPi0.root", "recreate");

  TH1F* hPi0s = new TH1F("hPi0s", "ECL Reconstructed Pi0 Energy for 1 GeV/c Pi0", 100, 0.2, 1.2);
  hPi0s->GetXaxis()->SetTitle("Pi0 Energy (GeV)");
  hPi0s->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed energy for 1 GeV Pi0s")); 
  hPi0s->GetListOfFunctions()->Add(new TNamed("Check","Should not be biased"));
  hPi0s->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  gPad->RedrawAxis();
  pi0_tree->Draw("eclPi0Energy>>hPi0s");
  hPi0s->Write();
  delete hPi0s;

  TH1F* hPi0mass = new TH1F("hPi0mass", "ECL combined Pi0 Mass for 1 GeV/c Pi0", 100, 0.08, 0.18);
  hPi0mass->GetXaxis()->SetTitle("Pi0 combined mass (GeV/c^{2})");
  hPi0mass->GetListOfFunctions()->Add(new TNamed("Description", 
    "Reconstructed mass for 1 GeV Pi0s")); 
  hPi0mass->GetListOfFunctions()->Add(new TNamed("Check", 
    "Should not be biased"));
  hPi0mass->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  gPad->RedrawAxis();
  pi0_tree->Draw("eclPi0Mass>>hPi0mass");
  hPi0mass->Write();
  delete hPi0mass;

  output->Close();
  }*/

void ECLPion(TTree* pion_tree){

  TFile* output = TFile::Open("ECLPion.root", "recreate");

  TH1F* hPionE = new TH1F("hPionE", "Cluster energy, 1 GeV charged pion", 100, 0., 1.2);
  pion_tree->Draw("eclClusterEnergy>>hPionE","eclClusterEnergy>0");
  hPionE->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hPionE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for single 1GeV charged pion")); 
  hPionE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be 0.2 GeV"));
  hPionE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  hPionE->Write();


  TH1F* hPionMultip = new TH1F("hPionMultip", "Cluster multiplicity, 1GeV charged pion", 10,0,10);
  pion_tree->Draw("eclClusterMultip>>hPionMultip","eclClusterMultip>0");
  hPionMultip->SetMaximum(600);
  hPionMultip->GetXaxis()->SetTitle("ECL cluster multiplicity");
  hPionMultip->GetListOfFunctions()->Add(new TNamed("Description","ECL cluster multiplicity for 1 GeV charged pion")); 
  hPionMultip->GetListOfFunctions()->Add(new TNamed("Check","Cluster multiplicity should be low"));
  hPionMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  hPionMultip->Write();
  
  output->Close();
  //delete output;
}

void ECLMuon(TTree* muon_tree)
{

  TFile* output = TFile::Open("ECLMuon.root", "recreate");

  TH1F* hMuonsE = new TH1F("hMuonsE", "Reconstructed cluster energy for 1GeV muons, the typical energy is 0.2 GeV", 100, 0., 1.2);
  muon_tree->Draw("eclClusterEnergy>>hMuonsE");
  hMuonsE->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Description","Energy release in the ECL for 1 GeV muons")); 
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 200 MeV"));
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  hMuonsE->Write();

  TH1F* hMuonsFake = new TH1F("hMuonsFake","Fake reconstructed gamma for 1000 muons", 10,0,10);
  hMuonsFake->SetMaximum(150);
  muon_tree->Draw("(eclClusterIdx)>>hMuonsFake","eclClusterIsTrack==0&&eclClusterToMc1==0");
  hMuonsFake->SetMaximum(150);
  hMuonsFake->GetXaxis()->SetTitle("Gamma Idx");
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Description", "Fake reconstructed gamma every 1000 muons")); 
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Check", "Should be below 5%"));
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Contact", "elisa.manoni@pg.infn.it")); 
  hMuonsFake->Write();

  //delete output;
  output->Close();
}


void ECLCluster(TTree* cluster_tree)
{

  TH1F* hMultip = new TH1F("hMultip","Reconstructed Cluster Multiplicity", 10, 0., 10.);
  cluster_tree->Draw("eclClusterMultip>>hMultip");
  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Multiplicity of reconstructed clusters for 500 MeV/c single photons"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("Cluster Multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 


  TH1F* hEnergy = new TH1F("hEnergy","Corrected Cluster Energy", 120, 0., 0.6);
  cluster_tree->Draw("eclClusterEnergy>>hEnergy");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Description", "Corrected deposited energy for 500 MeV/c single photons"));
  hEnergy->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 0.5 GeV and left-side tail.")); 
  hEnergy->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hEnDepSum = new TH1F("hEnDepSum","Reconstructed Cluster Energy", 120, 0., 0.6);
  cluster_tree->Draw("eclClusterEnergyDepSum>>hEnDepSum");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Description", "Uncorrected deposited energy for 500 MeV/c single photons"));
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 0.5 GeV and left-side tail.")); 
  hEnDepSum->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hHighestE = new TH1F("hHighestE","Highest Energy Deposit", 100, 0., 0.50);
  cluster_tree->Draw("eclClusterHighestE>>hHighestE");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Description", "Highest energy deposited in a crystal for 500 MeV/c single photons"));
  hHighestE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hHighestE->GetXaxis()->SetTitle("Deposited Energy(GeV)");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hE9oE25 = new TH1F("hE9oE25","E9/E25", 120, 0., 1.2);
  cluster_tree->Draw("eclClusterE9oE25>>hE9oE25");
  hE9oE25->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 500 MeV/c single photons"));
  hE9oE25->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE9oE25->GetXaxis()->SetTitle("E9oE25");
  hE9oE25->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hphi = new TH1F("hphi", "Reconstructed #phi Angle", 128, -3.2, 3.2);
  cluster_tree->Draw("eclClusterPhi>>hphi");
  hphi->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi angle of the cluster for 500 MeV/c single photons"));
  hphi->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape, flat distribution"));
  hphi->GetXaxis()->SetTitle("#phi (rad)");
  hphi->SetMinimum(.0);
  hphi->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TFile* output = TFile::Open("ECLCluster.root", "recreate");
  hMultip->Write();
  hEnergy->Write();
  hEnDepSum->Write();
  hHighestE->Write();
  hE9oE25->Write();
  hphi->Write();
  
  //delete output;
  output->Close();
}


void ECLBkg(TTree* bkg_tree)
{

  TFile* output = TFile::Open("ECLBkg.root", "recreate");

  TH1F* bkgClusterE = new TH1F("bkgClusterE", "Cluster energy, bkg only", 60, 0., 0.3);
  bkg_tree->Draw("eclClusterEnergy>>bkgClusterE","eclClusterEnergy>0");
  bkgClusterE->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for bkg clusters")); 
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be peaked at 0"));
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  bkgClusterE->Write();

  TH1F* bkgClusterMultip = new TH1F("bkgClusterMultip", "Cluster multiplicity bkg only", 100, 0, 100);
  bkg_tree->Draw("eclClusterMultip>>bkgClusterMultip","eclClusterMultip>0");
  bkgClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity Bkg");
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Description","ECL cluster multiplicity for bkg")); 
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Check","Cluster multiplicity should be around 50 (Jun 2014)"));
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgClusterMultip->Write();
 
  output->Close();
  // delete output;
}

void ECL2D(TTree* bkg_tree)
{

  TFile* output = TFile::Open("ECL2D.root", "recreate");

  gStyle->SetOptStat(000000);

  TH2F* BDyz = new TH2F("BDyz", "Cluster position", 200, -1.5, 2.5, 100, -1.5, 1.5);
  //TH2F* BDyz1 = new TH2F("BDyz1", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz2 = new TH2F("BDyz2", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz3 = new TH2F("BDyz3", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz4 = new TH2F("BDyz4", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz5 = new TH2F("BDyz5", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz6 = new TH2F("BDyz6", "", 200, -1.5, 2.5, 100, -1.5, 1.5);

  bkg_tree->Draw("(TMath::Tan(eclClusterTheta))*(1.9616):1.9616>>BDyz","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)&&(eclClusterPhi<0)");
  bkg_tree->Draw("1.250:(1.250*(TMath::Cos(eclClusterTheta))/((TMath::Sin(eclClusterTheta))))>>BDyz2","(eclClusterTheta>32.20*(3.1415/180))&&eclClusterTheta<(180-51.28)*(3.1415/180)&&(eclClusterPhi<0)");
  bkg_tree->Draw("(-TMath::Tan(eclClusterTheta))*(1.0216):-1.0216>>BDyz3","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)&&eclClusterPhi<(0)");
  bkg_tree->Draw("(-(TMath::Tan(eclClusterTheta))*(1.9616)):1.9616>>BDyz4","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)&&(eclClusterPhi>0)");
  bkg_tree->Draw("(-1.250):(1.250*(TMath::Cos(eclClusterTheta))/((TMath::Sin(eclClusterTheta))))>>BDyz5","(eclClusterTheta>32.20*(3.1415/180))&&eclClusterTheta<(180-51.28)*(3.1415/180)&&(eclClusterPhi>0)");
  bkg_tree->Draw("((TMath::Tan(eclClusterTheta))*(1.0216)):-1.0216>>BDyz6","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)&&(eclClusterPhi>0)");

  BDyz->Add(BDyz2, 1);
  BDyz->Add(BDyz3, 1);
  BDyz->Add(BDyz4, 1);
  BDyz->Add(BDyz5, 1);
  BDyz->Add(BDyz6, 1);
  BDyz->Draw("colz");

  BDyz->GetXaxis()->SetTitle("x (m)");
  BDyz->GetYaxis()->SetTitle("y (m)");
  BDyz->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in the ecl (upper part is phi<0, lower part phi>0)")); 
  BDyz->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz->Write();

  TH2F* BDyz7 = new TH2F("BDyz7", "Cluster position FWD", 200, -2, 2, 200, -1.5, 1.5);
  TH2F* BDyz8 = new TH2F("BDyz8", "Cluster position BWD", 200, -2, 2, 200, -1.5, 1.5);

  bkg_tree->Draw("(2.402*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(2.402*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz7","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)");
  BDyz7->Draw("colz");

  BDyz7->GetXaxis()->SetTitle("x (m)");
  BDyz7->GetYaxis()->SetTitle("y (m)");
  BDyz7->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz7->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in the FWD ecl")); 
  BDyz7->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz7->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz7->Write();

  bkg_tree->Draw("(1.669*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(1.669*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz8","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)");
  BDyz8->Draw("colz");

  BDyz8->GetXaxis()->SetTitle("x (m)");
  BDyz8->GetYaxis()->SetTitle("y (m)");
  BDyz8->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz8->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in the BWD ecl")); 
  BDyz8->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz8->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz8->Write();

  TH2F* BDyz9 = new TH2F("BDyz9", "Cluster position barrel", 200, -2, 2, 200, -1.5, 1.5);
  bkg_tree->Draw("(1.250*TMath::Cos(eclClusterPhi)):(1.250*TMath::Sin(eclClusterPhi))>>BDyz9","(32.20*(3.1415/180))<eclClusterTheta<(180-51.28)*(3.1415/180)");
  BDyz9->Draw("colz");
  BDyz9->GetXaxis()->SetTitle("x (m)");
  BDyz9->GetYaxis()->SetTitle("y (m)");
  BDyz9->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz9->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in barrel ecl")); 
  BDyz9->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz9->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz9->Write();
  
  output->Close();
  //delete output;
}

