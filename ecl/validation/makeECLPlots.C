/* makePlots.C 
 * ROOT macro for ECL validation plots 
 * Authors: Benjamin Oberhof, Elisa Manoni 
 * 2015-06
*/

/*
<header>
<input>ECLBkgOutput.root, ECLClusterOutputFWD.root, ECLClusterOutputBarrel.root, ECLClusterOutputBWD.root,ECLMuonOutput.root</input>
<output>ECLBkg.root, ECL2D.root, ECLMuon.root, ECLClusterFWD.root, ECLClusterBarrel.root, ECLClusterBWD.root, ECLCalDigitFWD.root, ECLCalDigitBarrel.root, ECLCalDigitBWD.root, ECLClusterResoBWD</output>
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
void ECLClusterFWD(TTree* cluster_treeFWD);
void ECLClusterBarrel(TTree* cluster_treeBarrel);
void ECLClusterBWD(TTree* cluster_treeBWD);
void ECLClusterResoFWD(TTree* cluster_treeFWD);
void ECLClusterResoBarrel(TTree* cluster_treeBarrel);
void ECLClusterResoBWD(TTree* cluster_treeBWD);
void ECLCalDigitFWD(TTree* cluster_treeFWD);
void ECLCalDigitBarrel(TTree* cluster_treeBarrel);
void ECLCalDigitBWD(TTree* cluster_treeBWD);
void ECLMuon(TTree* muon_tree);
Double_t Novosibirsk(Double_t *xp,Double_t *par);

void makeECLPlots() 
{
  
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));
  if (TFile::Open("../ECLBkgOutput.root") != NULL) {
    TFile* bkg_input = TFile::Open("../ECLBkgOutput.root");
    TTree* bkg_tree = (TTree*) bkg_input->Get("m_tree");
    ECL2D(bkg_tree);
    ECLBkg(bkg_tree);
  }
  if (TFile::Open("../ECLClusterOutputFWD.root") != NULL) {
    TFile* cluster_inputFWD = TFile::Open("../ECLClusterOutputFWD.root");
    TTree* cluster_treeFWD = (TTree*) cluster_inputFWD->Get("m_tree");
    ECLClusterFWD(cluster_treeFWD);
  }
  if (TFile::Open("../ECLClusterOutputFWD.root") != NULL) {
    TFile* clusterReso_inputFWD = TFile::Open("../ECLClusterOutputFWD.root");
    TTree* clusterReso_treeFWD = (TTree*) clusterReso_inputFWD->Get("m_tree");
    ECLClusterResoFWD(clusterReso_treeFWD);
  }
  if (TFile::Open("../ECLClusterOutputFWD.root") != NULL) {
    TFile* cd_inputFWD = TFile::Open("../ECLClusterOutputFWD.root");
    TTree* cd_treeFWD = (TTree*) cd_inputFWD->Get("m_tree");
    ECLCalDigitFWD(cd_treeFWD);
  } 
  if (TFile::Open("../ECLClusterOutputBarrel.root") != NULL) {
    TFile* cluster_inputBarrel = TFile::Open("../ECLClusterOutputBarrel.root");
    TTree* cluster_treeBarrel = (TTree*) cluster_inputBarrel->Get("m_tree");
    ECLClusterBarrel(cluster_treeBarrel);
  } 
  if (TFile::Open("../ECLClusterOutputBarrel.root") != NULL) {
    TFile* clusterReso_inputBarrel = TFile::Open("../ECLClusterOutputBarrel.root");
    TTree* clusterReso_treeBarrel = (TTree*) clusterReso_inputBarrel->Get("m_tree");
    ECLClusterResoBarrel(clusterReso_treeBarrel);
  }
  if (TFile::Open("../ECLClusterOutputBarrel.root") != NULL) {
    TFile* cd_inputBarrel = TFile::Open("../ECLClusterOutputBarrel.root");
    TTree* cd_treeBarrel = (TTree*) cd_inputBarrel->Get("m_tree");
    ECLCalDigitBarrel(cd_treeBarrel);
  }
  if (TFile::Open("../ECLClusterOutputBWD.root") != NULL) {
    TFile* cluster_inputBWD = TFile::Open("../ECLClusterOutputBWD.root");
    TTree* cluster_treeBWD = (TTree*) cluster_inputBWD->Get("m_tree");
    ECLClusterBWD(cluster_treeBWD);
  }
  if (TFile::Open("../ECLClusterOutputBWD.root") != NULL) {
    TFile* clusterReso_inputBWD = TFile::Open("../ECLClusterOutputBWD.root");
    TTree* clusterReso_treeBWD = (TTree*) clusterReso_inputBWD->Get("m_tree");
    ECLClusterResoBWD(clusterReso_treeBWD);
  }
  if (TFile::Open("../ECLClusterOutputBWD.root") != NULL) {
    TFile* cd_inputBWD = TFile::Open("../ECLClusterOutputBWD.root");
    TTree* cd_treeBWD = (TTree*) cd_inputBWD->Get("m_tree");
    ECLCalDigitBWD(cd_treeBWD);
  }
  if (TFile::Open("../ECLMuonOutput.root") != NULL) {
    TFile* muon_input = TFile::Open("../ECLMuonOutput.root");
    TTree* muon_tree = (TTree*) muon_input->Get("m_tree");
    ECLMuon(muon_tree);
  }

}


void ECLMuon(TTree* muon_tree)
{

  TFile* output = TFile::Open("ECLMuon.root", "recreate");

  TH1F* hMuonsE = new TH1F("hMuonsE", "Reconstructed cluster energy for (0.5 - 3 GeV) muons, the typical energy is 0.2 GeV", 100, 0., 0.6);
  muon_tree->Draw("eclClusterEnergy>>hMuonsE","eclClusterToMC1==0&&eclClusterHypothesisId==5&&(eclClusterToMCWeight1-eclClusterToBkgWeight)/eclClusterEnergy>0");
  hMuonsE->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Description","Energy release in the ECL for 1 GeV muons")); 
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 200 MeV"));
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  hMuonsE->Write();

  TH1F* hMuonsFake = new TH1F("hMuonsFake","Number of track-matched (right) and non-matched (non-bkg) clusters for 1000 generated muons", 20,-2,2);
  hMuonsFake->SetMaximum(1100);
  muon_tree->Draw("(2*eclClusterIsTrack-1)>>hMuonsFake","eclClusterToMC1==0&&eclClusterHypothesisId==5&&(eclClusterToMCWeight1-eclClusterToBkgWeight)>0");
  //hMuonsFake->Scale(0.1);
  hMuonsFake->GetXaxis()->SetTitle("Percentage of fake (non-bkg) neutral clusters");
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Description", "Number of track-matched (right) and non-matched (non-bkg) clusters for 1000 generated muons")); 
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Check", "Number of unmatched clusters should be around/below 5%"));
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Contact", "elisa.manoni@pg.infn.it")); 
  hMuonsFake->Write(); 

  TH1F* hMuonsFakeTheta = new TH1F("hMuonsFakeTheta","#theta distribution for fake (non-bkg) neutral clusters", 25, 0, 3.2);

  muon_tree->Draw("eclClusterTheta>>hMuonsFakeTheta","eclClusterToMC1==0&&eclClusterIsTrack==0&&eclClusterHypothesisId==5&&(eclClusterToMCWeight1-eclClusterToBkgWeight)>0");
  hMuonsFakeTheta->GetXaxis()->SetTitle("#theta (rad)");
  hMuonsFakeTheta->GetListOfFunctions()->Add(new TNamed("Description", "#theta distribution for fake gammas")); 
  hMuonsFakeTheta->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape. For this plot the statistic should be low but may help in problem diagnosis when the number of fake cluster is unexpectedly high."));
  hMuonsFakeTheta->GetListOfFunctions()->Add(new TNamed("Contact", "elisa.manoni@pg.infn.it")); 
  hMuonsFakeTheta->Write(); 

  TH1F* hMuonsFakePhi = new TH1F("hMuonsFakePhi","#phi distribution for fake (non-bkg) neutral clusters", 25, -3.6, 3.6);

  muon_tree->Draw("eclClusterPhi>>hMuonsFakePhi","eclClusterToMC1==0&&eclClusterIsTrack==0&&eclClusterHypothesisId==5&&(eclClusterToMCWeight1-eclClusterToBkgWeight)>0");
  hMuonsFakePhi->GetXaxis()->SetTitle("#phi (rad)");
  hMuonsFakePhi->GetListOfFunctions()->Add(new TNamed("Description", "#phi distribution for fake gammas")); 
  hMuonsFakePhi->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape. For this plot the statistic should be low but may help in prob\
lem diagnosis when the number of fake cluster is unexpectedly high."));
  hMuonsFakePhi->GetListOfFunctions()->Add(new TNamed("Contact", "elisa.manoni@pg.infn.it")); 
  hMuonsFakePhi->Write(); 

  output->Close();
  delete output;
}


void ECLClusterFWD(TTree* cluster_treeFWD)
{

  TH1F* hMultip = new TH1F("hMultip","Cluster Multiplicity", 10, 0., 10.);

  std::vector<int>* eclClusterHypothesisId=0;
  cluster_treeFWD->SetBranchAddress("eclClusterHypothesisId", &eclClusterHypothesisId);
  std::vector<int>* eclClusterToMC1=0;
  cluster_treeFWD->SetBranchAddress("eclClusterToMC1", &eclClusterToMC1);

  for(int i=0; i<cluster_treeFWD->GetEntries();i++){
    cluster_treeFWD->GetEntry(i);
    int h=0;
    for(int j=0;j<eclClusterHypothesisId->size();j++){
      if (eclClusterHypothesisId->at(j) == 5)
	if (eclClusterToMC1->at(j) == 0)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Multiplicity of (true) reconstructed clusters for 100 MeV/c single photons in FWD endcap"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("Cluster multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 


  TH1F* hEnergy = new TH1F("hEnergy","Corrected Cluster Energy in FWD endcap", 100, 0., 0.2);
  cluster_treeFWD->Draw("eclClusterEnergy>>hEnergy","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Description", "Corrected deposited energy for 100 MeV/c single photons in FWD endcap"));
  hEnergy->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnergy->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hEnDepSum = new TH1F("hEnDepSum","Reconstructed Cluster Energy in FWD endcap", 100, 0., 0.2);
  cluster_treeFWD->Draw("eclClusterEnergyDepSum>>hEnDepSum","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Description", "Uncorrected deposited energy for 100 MeV/c single photons in FWD endcap"));
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnDepSum->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hECorr = new TH1F("hECorr","Cluster Energy correction in FWD endcap", 100, -0.01, 0.01);
  cluster_treeFWD->Draw("(eclClusterEnergy - eclClusterEnergyDepSum)>>hECorr","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hECorr->GetListOfFunctions()->Add(new TNamed("Description", "Energy corerction for 100 MeV/c single photons in FWD endcap"));
  hECorr->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hECorr->GetXaxis()->SetTitle("Energy correction (GeV)");
  hECorr->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hHighestE = new TH1F("hHighestE","Highest Energy Deposit in FWD endcap", 100, 0., 0.14);
  cluster_treeFWD->Draw("eclClusterHighestE>>hHighestE","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Description", "Highest energy deposited in a crystal for 100 MeV/c single photons in FWD endcap"));
  hHighestE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hHighestE->GetXaxis()->SetTitle("Deposited Energy(GeV)");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hE9oE21 = new TH1F("hE9oE21","E9/E21 in FWD endcap", 120, 0., 1.2);
  cluster_treeFWD->Draw("eclClusterE9oE21>>hE9oE21","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in FWD endcap"));
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE9oE21->GetXaxis()->SetTitle("E9oE21");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hE1oE9 = new TH1F("hE1oE9","E1/E9 in FWD endcap", 120, 0., 1.2);
  cluster_treeFWD->Draw("eclClusterE1oE9>>hE1oE9","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in FWD endcap"));
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE1oE9->GetXaxis()->SetTitle("E1oE9");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));


  TH1F* hphi = new TH1F("hphi", "Reconstructed #phi Angle in FWD endcap", 64, -3.2, 3.2);
  cluster_treeFWD->Draw("eclClusterPhi>>hphi","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphi->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi angle of the cluster for 100 MeV/c single photons in FWD endcap"));
  hphi->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape, flat distribution"));
  hphi->GetXaxis()->SetTitle("#phi (rad)");
  hphi->SetMinimum(.0);
  hphi->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hEError = new TH1F("hEError", "Energy error in FWD endcap", 100, 0, 0.01);
  cluster_treeFWD->Draw("eclCluster Energy Error>>hEError","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEError->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed energy error of the cluster for 100 MeV/c single photons in FWD endcap"));
  hEError->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hEError->GetXaxis()->SetTitle("Cluster energy error (GeV)");
  hEError->SetMinimum(.0);
  hEError->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* htheta = new TH1F("htheta", "Reconstructed #theta Angle in FWD endcap", 64, 0.2, 0.6);
  cluster_treeFWD->Draw("eclClusterTheta>>htheta","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  htheta->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta angle of the cluster for 100 MeV/c single photons in FWD endcap"));
  htheta->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  htheta->GetXaxis()->SetTitle("#theta (rad)");
  htheta->SetMinimum(.0);
  htheta->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hthetaerror = new TH1F("hthetaerror", "Reconstructed #theta error in FWD endcap", 100, 0, 0.025);
  cluster_treeFWD->Draw("eclClusterThetaError>>hthetaerror","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta error of the cluster for 100 MeV/c single photons in FWD endcap"));
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hthetaerror->GetXaxis()->SetTitle("#theta error (rad)");
  hthetaerror->SetMinimum(.0);
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hphierror = new TH1F("hphierror", "Reconstructed #phi error in FWD endcap", 100, 0, 0.025);
  cluster_treeFWD->Draw("eclClusterPhiError>>hphierror","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphierror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi error of the cluster for 100 MeV/c single photons in FWD endcap"));
  hphierror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hphierror->GetXaxis()->SetTitle("#phi error (rad)");
  hphierror->SetMinimum(.0);
  hphierror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hR = new TH1F("hR", "Reconstructed #R in FWD endcap", 130, 210, 250);
  cluster_treeFWD->Draw("eclClusterR>>hR","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hR->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #R of the cluster for 100 MeV/c single photons in FWD endcap"));
  hR->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hR->GetXaxis()->SetTitle("#R ");
  hR->SetMinimum(.0);
  hR->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hTiming = new TH1F("hTiming", "Cluster Timing in FWD endcap", 130, -130, 130);
  cluster_treeFWD->Draw("eclClusterTiming>>hTiming","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hTiming->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in FWD endcap"));
  hTiming->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hTiming->GetXaxis()->SetTitle("#Timing");
  hTiming->SetMinimum(.0);
  hTiming->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hLAT = new TH1F("hLAT", "Cluster LAT in FWD endcap", 100, 0, 1);
  cluster_treeFWD->Draw("eclClusterLAT>>hLAT","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hLAT->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in FWD endcap"));
  hLAT->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hLAT->GetXaxis()->SetTitle("#LAT");
  hLAT->SetMinimum(.0);
  hLAT->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hNofCrystals = new TH1F("hNofCrystals", "Cluster NofCrystals in FWD endcap", 25, 0, 25);
  cluster_treeFWD->Draw("eclClusterNofCrystals>>hNofCrystals","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in FWD endcap"));
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hNofCrystals->GetXaxis()->SetTitle("#NofCrystals");
  hNofCrystals->SetMinimum(.0);
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hIsTrack = new TH1F("hIsTrack", "Cluster IsTrack in FWD endcap", 2, 0, 1);
  cluster_treeFWD->Draw("eclClusterIsTrack>>hIsTrack","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in FWD endcap"));
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hIsTrack->GetXaxis()->SetTitle("#IsTrack");
  hIsTrack->SetMinimum(.0);
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike40 = new TH1F("hAbsZernike40", "Cluster AbsZernike40 in FWD endcap", 100, 0, 2);
  cluster_treeFWD->Draw("eclClusterAbsZernike40>>hAbsZernike40","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike40 for 100 MeV/c single photons in FWD endcap"));
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike40->GetXaxis()->SetTitle("AbsZernike40");
  hAbsZernike40->SetMinimum(.0);
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike51 = new TH1F("hAbsZernike51", "Cluster AbsZernike51 in FWD endcap", 100, 0, 1.2);
  cluster_treeFWD->Draw("eclClusterAbsZernike51>>hAbsZernike51","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in FWD endcap"));
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike51->GetXaxis()->SetTitle("AbsZernike51");
  hAbsZernike51->SetMinimum(.0);
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hZernikeMVA = new TH1F("hZernikeMVA", "Cluster ZernikeMVA in FWD endcap", 100, 0, 1.2);
  cluster_treeFWD->Draw("eclClusterZernikeMVA>>hZernikeMVA","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Description", "Cluster ZernikeMVA for 100 MeV/c single photons in FWD endcap"));
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hZernikeMVA->GetXaxis()->SetTitle("ZernikeMVA");
  hZernikeMVA->SetMinimum(.0);
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hSecondMoment = new TH1F("hSecondMoment", "Cluster Second Moment in FWD endcap", 100, 0, 8);
  cluster_treeFWD->Draw("eclClusterSecondMoment>>hSecondMoment","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in FWD endcap"));
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hSecondMoment->GetXaxis()->SetTitle("SecondMoment");
  hSecondMoment->SetMinimum(.0);
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TFile* output = TFile::Open("ECLClusterFWD.root", "recreate");
  hMultip->Write();
  hEnergy->Write();
  hEnDepSum->Write();
  hHighestE->Write();
  hE9oE21->Write();
  hE1oE9->Write();
  hphi->Write();
  hEError->Write();
  htheta->Write();
  hthetaerror->Write();
  hphierror->Write();
  hR->Write();
  hTiming->Write();
  hLAT->Write();
  hNofCrystals->Write(); 
  hAbsZernike40->Write(); 
  hAbsZernike51->Write(); 
  hZernikeMVA->Write(); 
  hSecondMoment->Write(); 
  hECorr->Write();

  output->Close();
  delete output;
  delete hMultip;
  delete hEnergy;
  delete hEnDepSum;
  delete hHighestE;
  delete hE9oE21;
  delete hE1oE9;
  delete hphi;
  delete hEError;
  delete htheta;
  delete hthetaerror;
  delete hphierror;
  delete hR;
  delete hTiming;
  delete hLAT;
  delete hNofCrystals;
  delete hIsTrack;
  delete hAbsZernike40; 
  delete hAbsZernike51; 
  delete hZernikeMVA; 
  delete hSecondMoment;
  delete hECorr;

}

void ECLClusterBarrel(TTree* cluster_treeBarrel)
{


  TH1F* hMultip = new TH1F("hMultip","Reconstructed Cluster Multiplicity in Barrel", 10, 0., 10.);

  std::vector<int>* eclClusterHypothesisId=0;
  cluster_treeBarrel->SetBranchAddress("eclClusterHypothesisId", &eclClusterHypothesisId);
  std::vector<int>* eclClusterToMC1=0;
  cluster_treeBarrel->SetBranchAddress("eclClusterToMC1", &eclClusterToMC1);

  for(int i=0; i<cluster_treeBarrel->GetEntries();i++){
    cluster_treeBarrel->GetEntry(i);
    int h=0;
    for(int j=0;j<eclClusterHypothesisId->size();j++){
      if (eclClusterHypothesisId->at(j) == 5)
	if (eclClusterToMC1->at(j) == 0)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Multiplicity of reconstructed clusters for 100 MeV/c single photons in Barrel"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("Cluster multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hEnergy = new TH1F("hEnergy","Corrected Cluster Energy in Barrel", 100, 0., 0.2);
  cluster_treeBarrel->Draw("eclClusterEnergy>>hEnergy","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Description", "Corrected deposited energy for 100 MeV/c single photons in Barrel"));
  hEnergy->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnergy->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hEnDepSum = new TH1F("hEnDepSum","Reconstructed Cluster Energy in Barrel", 100, 0., 0.2);
  cluster_treeBarrel->Draw("eclClusterEnergyDepSum>>hEnDepSum","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Description", "Uncorrected deposited energy for 100 MeV/c single photons in Barrel"));
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnDepSum->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hECorr = new TH1F("hECorr","Cluster Energy correction in Barrel", 100, -0.01, 0.01);
  cluster_treeBarrel->Draw("(eclClusterEnergy - eclClusterEnergyDepSum)>>hECorr","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hECorr->GetListOfFunctions()->Add(new TNamed("Description", "Energy corerction for 100 MeV/c single photons in barrel"));
  hECorr->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hECorr->GetXaxis()->SetTitle("Energy correction (GeV)");
  hECorr->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hHighestE = new TH1F("hHighestE","Highest Energy Deposit in Barrel", 100, 0., 0.14);
  cluster_treeBarrel->Draw("eclClusterHighestE>>hHighestE","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Description", "Highest energy deposited in a crystal for 100 MeV/c single photons in Barrel"));
  hHighestE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hHighestE->GetXaxis()->SetTitle("Deposited energy(GeV)");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hE9oE21 = new TH1F("hE9oE21","E9/E21 in Barrel", 120, 0., 1.2);
  cluster_treeBarrel->Draw("eclClusterE9oE21>>hE9oE21","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in Barrel"));
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE9oE21->GetXaxis()->SetTitle("E9oE21");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hE1oE9 = new TH1F("hE1oE9","E1/E9 in Barrel", 120, 0., 1.2);
  cluster_treeBarrel->Draw("eclClusterE1oE9>>hE1oE9","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in Barrel"));
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE1oE9->GetXaxis()->SetTitle("E1oE9");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hphi = new TH1F("hphi", "Reconstructed #phi Angle in Barrel", 64, -3.2, 3.2);
  cluster_treeBarrel->Draw("eclClusterPhi>>hphi","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphi->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi angle of the cluster for 100 MeV/c single photons in Barrel"));
  hphi->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape, flat distribution"));
  hphi->GetXaxis()->SetTitle("#phi (rad)");
  hphi->SetMinimum(.0);
  hphi->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hEError = new TH1F("hEError", "Energy error in Barrel", 100, 0, 0.01);
  cluster_treeBarrel->Draw("eclCluster Energy Error>>hEError","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEError->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed energy error of the cluster for 100 MeV/c single photons in Barrel"));
  hEError->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hEError->GetXaxis()->SetTitle("Cluster energy error (GeV)");
  hEError->SetMinimum(.0);
  hEError->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* htheta = new TH1F("htheta", "Reconstructed #theta Angle in Barrel", 64, 3.2/4, 3.2*(3/4));
  cluster_treeBarrel->Draw("eclClusterTheta>>htheta","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  htheta->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta angle of the cluster for 100 MeV/c single photons in Barrel"));
  htheta->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  htheta->GetXaxis()->SetTitle("#theta (rad)");
  htheta->SetMinimum(.0);
  htheta->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hthetaerror = new TH1F("hthetaerror", "Reconstructed #theta error in Barrel", 100, 0, 0.025);
  cluster_treeBarrel->Draw("eclClusterThetaError>>hthetaerror","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta error of the cluster for 100 MeV/c single photons in Barrel"));
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hthetaerror->GetXaxis()->SetTitle("#theta error (rad)");
  hthetaerror->SetMinimum(.0);
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hphierror = new TH1F("hphierror", "Reconstructed #phi error in Barrel", 100, 0, 0.025);
  cluster_treeBarrel->Draw("eclClusterPhiError>>hphierror","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphierror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi error of the cluster for 100 MeV/c single photons in Barrel"));
  hphierror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hphierror->GetXaxis()->SetTitle("#phi error (rad)");
  hphierror->SetMinimum(.0);
  hphierror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hR = new TH1F("hR", "Reconstructed #R in Barrel", 130, 130, 260);
  cluster_treeBarrel->Draw("eclClusterR>>hR","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hR->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #R of the cluster for 100 MeV/c single photons in Barrel"));
  hR->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hR->GetXaxis()->SetTitle("#R ");
  hR->SetMinimum(.0);
  hR->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hTiming = new TH1F("hTiming", "Cluster Timing in Barrel", 130, -130, 130);
  cluster_treeBarrel->Draw("eclClusterTiming>>hTiming","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hTiming->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in Barrel"));
  hTiming->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hTiming->GetXaxis()->SetTitle("#Timing");
  hTiming->SetMinimum(.0);
  hTiming->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hLAT = new TH1F("hLAT", "Cluster LAT in Barrel", 100, 0, 1);
  cluster_treeBarrel->Draw("eclClusterLAT>>hLAT","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hLAT->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in Barrel"));
  hLAT->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hLAT->GetXaxis()->SetTitle("#LAT");
  hLAT->SetMinimum(.0);
  hLAT->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hNofCrystals = new TH1F("hNofCrystals", "Cluster NofCrystals in Barrel", 25, 0, 25);
  cluster_treeBarrel->Draw("eclClusterNofCrystals>>hNofCrystals","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in Barrel"));
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hNofCrystals->GetXaxis()->SetTitle("#NofCrystals");
  hNofCrystals->SetMinimum(.0);
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hIsTrack = new TH1F("hIsTrack", "Cluster IsTrack in Barrel", 2, 0, 1);
  cluster_treeBarrel->Draw("eclClusterIsTrack>>hIsTrack","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in Barrel"));
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hIsTrack->GetXaxis()->SetTitle("#IsTrack");
  hIsTrack->SetMinimum(.0);
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike40 = new TH1F("hAbsZernike40", "Cluster AbsZernike40 in Barrel ", 100, 0, 2);
  cluster_treeBarrel->Draw("eclClusterAbsZernike40>>hAbsZernike40","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike40 for 100 MeV/c single photons in Barrel "));
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike40->GetXaxis()->SetTitle("AbsZernike40");
  hAbsZernike40->SetMinimum(.0);
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike51 = new TH1F("hAbsZernike51", "Cluster AbsZernike51 in Barrel ", 100, 0, 1.2);
  cluster_treeBarrel->Draw("eclClusterAbsZernike51>>hAbsZernike51","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in Barrel "));
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike51->GetXaxis()->SetTitle("AbsZernike51");
  hAbsZernike51->SetMinimum(.0);
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hZernikeMVA = new TH1F("hZernikeMVA", "Cluster ZernikeMVA in Barrel ", 100, 0, 1.2);
  cluster_treeBarrel->Draw("eclClusterZernikeMVA>>hZernikeMVA","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Description", "Cluster ZernikeMVA for 100 MeV/c single photons in Barrel "));
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hZernikeMVA->GetXaxis()->SetTitle("ZernikeMVA");
  hZernikeMVA->SetMinimum(.0);
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hSecondMoment = new TH1F("hSecondMoment", "Cluster Second Moment in Barrel ", 100, 0, 8);
  cluster_treeBarrel->Draw("eclClusterSecondMoment>>hSecondMoment","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in Barrel "));
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hSecondMoment->GetXaxis()->SetTitle("SecondMoment");
  hSecondMoment->SetMinimum(.0);
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TFile* output = TFile::Open("ECLClusterBarrel.root", "recreate");
  hMultip->Write();
  hEnergy->Write();
  hEnDepSum->Write();
  hHighestE->Write();
  hE9oE21->Write();
  hE1oE9->Write();
  hphi->Write();
  hEError->Write();
  htheta->Write();
  hthetaerror->Write();
  hphierror->Write();
  hR->Write();
  hTiming->Write();
  hLAT->Write();
  hNofCrystals->Write();  
  hAbsZernike40->Write(); 
  hAbsZernike51->Write(); 
  hZernikeMVA->Write(); 
  hSecondMoment->Write();
  hECorr->Write();

  output->Close();
  delete output;
  delete hMultip;
  delete hEnergy;
  delete hEnDepSum;
  delete hHighestE;
  delete hE9oE21;
  delete hE1oE9;
  delete hphi;
  delete hEError;
  delete htheta;
  delete hthetaerror;
  delete hphierror;
  delete hR;
  delete hTiming;
  delete hLAT;
  delete hNofCrystals;
  delete hIsTrack;
  delete hAbsZernike40; 
  delete hAbsZernike51; 
  delete hZernikeMVA; 
  delete hSecondMoment;
  delete hECorr;
}


void ECLClusterBWD(TTree* cluster_treeBWD)
{
  
  TH1F* hMultip = new TH1F("hMultip","Reconstructed Cluster Multiplicity in BWD endcap", 10, 0., 10.);

  std::vector<int>* eclClusterHypothesisId=0;
  cluster_treeBWD->SetBranchAddress("eclClusterHypothesisId", &eclClusterHypothesisId);
  std::vector<int>* eclClusterToMC1=0;
  cluster_treeBWD->SetBranchAddress("eclClusterToMC1", &eclClusterToMC1);

  for(int i=0; i<cluster_treeBWD->GetEntries();i++){
    cluster_treeBWD->GetEntry(i);
    int h=0;
    for(int j=0;j<eclClusterHypothesisId->size();j++){
      if (eclClusterHypothesisId->at(j) == 5)
	if (eclClusterToMC1->at(j) == 0)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Multiplicity of reconstructed clusters for 100 MeV/c single photons in BWD endcap"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("Cluster multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hEnergy = new TH1F("hEnergy","Corrected Cluster Energy in BWD endcap", 100, 0., 0.2);
  cluster_treeBWD->Draw("eclClusterEnergy>>hEnergy","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Description", "Corrected deposited energy for 100 MeV/c single photons in BWD endcap"));
  hEnergy->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnergy->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnergy->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hEnDepSum = new TH1F("hEnDepSum","Reconstructed Cluster Energy in BWD endcap", 100, 0., 0.2);
  cluster_treeBWD->Draw("eclClusterEnergyDepSum>>hEnDepSum","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Description", "Uncorrected deposited energy for 100 MeV/c single photons in BWD endcap"));
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 100 MeV and left-side tail.")); 
  hEnDepSum->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hEnDepSum->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hECorr = new TH1F("hECorr","Cluster Energy correction in BWD endcap", 100, -0.01, 0.01);
  cluster_treeBWD->Draw("(eclClusterEnergy - eclClusterEnergyDepSum)>>hECorr","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hECorr->GetListOfFunctions()->Add(new TNamed("Description", "Energy corerction for 100 MeV/c single photons in BWD endcap"));
  hECorr->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hECorr->GetXaxis()->SetTitle("Energy correction (GeV)");
  hECorr->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hHighestE = new TH1F("hHighestE","Highest Energy Deposit in BWD endcap", 100, 0., 0.14);
  cluster_treeBWD->Draw("eclClusterHighestE>>hHighestE","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Description", "Highest energy deposited in a crystal for 100 MeV/c single photons in BWD endcap"));
  hHighestE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hHighestE->GetXaxis()->SetTitle("Deposited energy(GeV)");
  hHighestE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hE9oE21 = new TH1F("hE9oE21","E9/E21 in BWD endcap", 120, 0., 1.2);
  cluster_treeBWD->Draw("eclClusterE9oE21>>hE9oE21","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in BWD endcap"));
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE9oE21->GetXaxis()->SetTitle("E9oE21");
  hE9oE21->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hE1oE9 = new TH1F("hE1oE9","E1/E9 in BWD endcap", 120, 0., 1.2);
  cluster_treeBWD->Draw("eclClusterE1oE9>>hE1oE9","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 100 MeV/c single photons in BWD endcap"));
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE1oE9->GetXaxis()->SetTitle("E1oE9");
  hE1oE9->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hphi = new TH1F("hphi", "Reconstructed #phi Angle in BWD endcap", 64, -3.2, 3.2);
  cluster_treeBWD->Draw("eclClusterPhi>>hphi","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphi->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi angle of the cluster for 100 MeV/c single photons in BWD endcap"));
  hphi->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape, flat distribution"));
  hphi->GetXaxis()->SetTitle("#phi (rad)");
  hphi->SetMinimum(.0);
  hphi->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hEError = new TH1F("hEError", "Energy error in BWD endcap", 100, 0, 0.015);
  cluster_treeBWD->Draw("eclCluster Energy Error>>hEError","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hEError->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed energy error of the cluster for 100 MeV/c single photons in BWD endcap"));
  hEError->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hEError->GetXaxis()->SetTitle("Cluster energy error (GeV)");
  hEError->SetMinimum(.0);
  hEError->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* htheta = new TH1F("htheta", "Reconstructed #theta Angle in BWD endcap", 64, 2.2, 2.8);
  cluster_treeBWD->Draw("eclClusterTheta>>htheta","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  htheta->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta angle of the cluster for 100 MeV/c single photons in BWD endcap"));
  htheta->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  htheta->GetXaxis()->SetTitle("#theta (rad)");
  htheta->SetMinimum(.0);
  htheta->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hthetaerror = new TH1F("hthetaerror", "Reconstructed #theta error in BWD endcap", 100, 0, 0.025);
  cluster_treeBWD->Draw("eclClusterThetaError>>hthetaerror","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #theta error of the cluster for 100 MeV/c single photons in BWD endcap"));
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hthetaerror->GetXaxis()->SetTitle("#theta error (rad)");
  hthetaerror->SetMinimum(.0);
  hthetaerror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hphierror = new TH1F("hphierror", "Reconstructed #phi error in BWD endcap", 100, 0, 0.025);
  cluster_treeBWD->Draw("eclClusterPhiError>>hphierror","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hphierror->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi error of the cluster for 100 MeV/c single photons in BWD endcap"));
  hphierror->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hphierror->GetXaxis()->SetTitle("#phi error (rad)");
  hphierror->SetMinimum(.0);
  hphierror->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hR = new TH1F("hR", "Reconstructed #R in BWD endcap", 130, 120, 180);
  cluster_treeBWD->Draw("eclClusterR>>hR","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hR->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #R of the cluster for 100 MeV/c single photons in BWD endcap"));
  hR->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hR->GetXaxis()->SetTitle("#R ");
  hR->SetMinimum(.0);
  hR->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hTiming = new TH1F("hTiming", "Cluster Timing in BWD endcap", 130, -130, 130);
  cluster_treeBWD->Draw("eclClusterTiming>>hTiming","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hTiming->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in BWD endcap"));
  hTiming->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hTiming->GetXaxis()->SetTitle("#Timing");
  hTiming->SetMinimum(.0);
  hTiming->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hLAT = new TH1F("hLAT", "Cluster LAT in BWD endcap", 100, 0, 1);
  cluster_treeBWD->Draw("eclClusterLAT>>hLAT","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hLAT->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in BWD endcap"));
  hLAT->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hLAT->GetXaxis()->SetTitle("#LAT");
  hLAT->SetMinimum(.0);
  hLAT->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hNofCrystals = new TH1F("hNofCrystals", "Cluster NofCrystals in BWD endcap", 25, 0, 25);
  cluster_treeBWD->Draw("eclClusterNofCrystals>>hNofCrystals","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in BWD endcap"));
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hNofCrystals->GetXaxis()->SetTitle("#NofCrystals");
  hNofCrystals->SetMinimum(.0);
  hNofCrystals->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hIsTrack = new TH1F("hIsTrack", "Cluster IsTrack in BWD endcap", 2, 0, 1);
  cluster_treeBWD->Draw("eclClusterIsTrack>>hIsTrack","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Description", "Cluster timing for 100 MeV/c single photons in BWD endcap"));
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hIsTrack->GetXaxis()->SetTitle("#IsTrack");
  hIsTrack->SetMinimum(.0);
  hIsTrack->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike40 = new TH1F("hAbsZernike40", "Cluster AbsZernike40 in BWD endcap", 100, 0, 2);
  cluster_treeBWD->Draw("eclClusterAbsZernike40>>hAbsZernike40","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike40 for 100 MeV/c single photons in BWD endcap"));
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike40->GetXaxis()->SetTitle("AbsZernike40");
  hAbsZernike40->SetMinimum(.0);
  hAbsZernike40->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hAbsZernike51 = new TH1F("hAbsZernike51", "Cluster AbsZernike51 in BWD endcap", 100, 0, 1.2);
  cluster_treeBWD->Draw("eclClusterAbsZernike51>>hAbsZernike51","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in BWD endcap"));
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hAbsZernike51->GetXaxis()->SetTitle("AbsZernike51");
  hAbsZernike51->SetMinimum(.0);
  hAbsZernike51->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hZernikeMVA = new TH1F("hZernikeMVA", "Cluster ZernikeMVA in BWD endcap", 100, 0, 1.2);
  cluster_treeBWD->Draw("eclClusterZernikeMVA>>hZernikeMVA","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Description", "Cluster ZernikeMVA for 100 MeV/c single photons in BWD endcap"));
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hZernikeMVA->GetXaxis()->SetTitle("ZernikeMVA");
  hZernikeMVA->SetMinimum(.0);
  hZernikeMVA->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TH1F* hSecondMoment = new TH1F("hSecondMoment", "Cluster Second Moment in BWD endcap", 100, 0, 8);
  cluster_treeBWD->Draw("eclClusterSecondMoment>>hSecondMoment","eclClusterTheta>131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5");
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Description", "Cluster Zernike51 for 100 MeV/c single photons in BWD endcap"));
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape"));
  hSecondMoment->GetXaxis()->SetTitle("SecondMoment");
  hSecondMoment->SetMinimum(.0);
  hSecondMoment->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));

  TFile* output = TFile::Open("ECLClusterBWD.root", "recreate");
  hMultip->Write();
  hEnergy->Write();
  hEnDepSum->Write();
  hHighestE->Write();
  hE9oE21->Write();
  hE1oE9->Write();
  hphi->Write();
  hEError->Write();
  htheta->Write();
  hthetaerror->Write();
  hphierror->Write();
  hR->Write();
  hTiming->Write();
  hLAT->Write();
  hNofCrystals->Write();  
  hAbsZernike40->Write(); 
  hAbsZernike51->Write(); 
  hZernikeMVA->Write(); 
  hSecondMoment->Write();
  hECorr->Write();

  output->Close();
  delete output;
  delete hMultip;
  delete hEnergy;
  delete hEnDepSum;
  delete hHighestE;
  delete hE9oE21;
  delete hE1oE9;
  delete hphi;
  delete hEError;
  delete htheta;
  delete hthetaerror;
  delete hphierror;
  delete hR;
  delete hTiming;
  delete hLAT;
  delete hNofCrystals;
  delete hIsTrack;
  delete hAbsZernike40; 
  delete hAbsZernike51; 
  delete hZernikeMVA; 
  delete hSecondMoment;
  delete hECorr;

}


void ECLClusterResoFWD(TTree* clusterReso_treeFWD)
{

  TH1F* hEnergyReso = new TH1F("hEnergyReso","Cluster EnergyReso in FWD endcap", 100, -0.1, 0.1);
  clusterReso_treeFWD->Draw("(eclClusterEnergy-0.1)>>hEnergyReso","eclClusterTheta<31.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5&&eclClusterEnergy>0.05");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Description", "Energy resolution for single photons in FWD endcap, minimum cluster energy 50 MeV"));
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Check", "Consistent resolution")); 
  hEnergyReso->GetXaxis()->SetTitle("Reconstructed cluster energy -  Generated energy (GeV)");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  hEnergyReso->Fit("gaus");
  gStyle->SetOptFit(11111);

  TFile* output = TFile::Open("ECLClusterResoFWD.root", "recreate");
  hEnergyReso->Write();
  
  output->Close();
  delete output;
  delete hEnergyReso;
}

void ECLClusterResoBarrel(TTree* clusterReso_treeBarrel)
{

  TH1F* hEnergyReso = new TH1F("hEnergyReso","Cluster EnergyReso in Barrel", 100, -0.1, 0.1);
  clusterReso_treeBarrel->Draw("(eclClusterEnergy-0.1)>>hEnergyReso","eclClusterTheta>31.5*3.1415/180&&eclClusterTheta<131.5*3.1415/180&&eclClusterToMC1==0&&eclClusterHypothesisId==5&&eclClusterEnergy>0.05");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Description", "Energy resolution for single photons in Barrel, minimum cluster energy 50 MeV"));
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Check", "Consistent resolution")); 
  hEnergyReso->GetXaxis()->SetTitle("Reconstructed cluster energy -  Generated energy (GeV)");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  hEnergyReso->Fit("gaus");
  gStyle->SetOptFit(11111);

  TFile* output = TFile::Open("ECLClusterResoBarrel.root", "recreate");
  hEnergyReso->Write();
  
  output->Close();
  delete output;
  delete hEnergyReso;
}

void ECLClusterResoBWD(TTree* clusterReso_treeBWD)
{
  TH1F* hEnergyReso = new TH1F("hEnergyReso","Cluster EnergyReso in BWD endcap", 100, -0.1, 0.1);
  clusterReso_treeBWD->Draw("(eclClusterEnergy-0.1)>>hEnergyReso","eclClusterTheta>131.5*(3.1415/180)&&eclClusterToMC1==0&&eclClusterHypothesisId==5&&eclClusterEnergy>0.05");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Description", "Energy resolution for single photons in BWD endcap, minimum cluster energy 50 MeV"));
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Check", "Consistent resolution")); 
  hEnergyReso->GetXaxis()->SetTitle("Reconstructed cluster energy -  Generated energy (GeV)");
  hEnergyReso->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  hEnergyReso->Fit("gaus");
  gStyle->SetOptFit(11111);

  TFile* output = TFile::Open("ECLClusterResoBWD.root", "recreate");
  hEnergyReso->Write();
  
  output->Close();
  delete output;
  delete hEnergyReso;
}

void ECLCalDigitFWD(TTree* cd_treeFWD)
{

  std::vector<int>* eclCalDigitToMC1=0;
  cd_treeFWD->SetBranchAddress("eclCalDigitToMC1", &eclCalDigitToMC1);
  std::vector<int>* eclCalDigitCellId=0;
  cd_treeFWD->SetBranchAddress("eclCalDigitCellId", &eclCalDigitCellId);

  TH1F* hMultip = new TH1F("hMultip","CalDigit Multiplicity in FWD endcap", 30, 0., 30.);

  for(int i=0; i<cd_treeFWD->GetEntries(); i++){
    cd_treeFWD->GetEntry(i);
    int h=0;
    for(int j=0;j<eclCalDigitToMC1->size();j++){
      if (eclCalDigitToMC1->at(j)==0)
	if (eclCalDigitCellId->at(j)<1153)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Cal Digit multiplicity for 100 MeV/c single photons in FWD endcap"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("CalDigit multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hAmp = new TH1F("hAmp","CalDigit Amplitude in FWD endcap", 100, 0., 0.04);
  cd_treeFWD->Draw("eclCalDigitAmp>>hAmp","eclCalDigitCellId<1153&&eclCalDigitToMC1==0");
  hAmp->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit amplitude for 100 MeV/c single photons in FWD endcap"));
  hAmp->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hAmp->GetXaxis()->SetTitle("CalDigit amplitude");
  hAmp->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hTimeFit = new TH1F("hTimeFit","CalDigit TimeFit in FWD endcap", 110, -1100., 1100.);
  cd_treeFWD->Draw("eclCalDigitTimeFit>>hTimeFit","eclCalDigitCellId<1153&&eclCalDigitToMC1==0");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit time fit for 100 MeV/c single photons in FWD endcap"));
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hTimeFit->GetXaxis()->SetTitle("CalDigit time fit");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hFitQuality = new TH1F("hFitQuality","CalDigit FitQuality in FWD endcap", 2, 0., 2.);
  hFitQuality->SetMinimum(0);
  cd_treeFWD->Draw("eclCalDigitFitQuality>>hFitQuality","eclCalDigitCellId<1153&&eclCalDigitToMC1==0");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit time fit quality for 100 MeV/c single photons in FWD endcap"));
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hFitQuality->GetXaxis()->SetTitle("CalDigit time fit quality");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hCellId = new TH1F("hCellId","CalDigit CellId in FWD endcap", 288, 0., 1153.);
  hCellId->SetMinimum(0);
  cd_treeFWD->Draw("eclCalDigitCellId>>hCellId","eclCalDigitCellId<1153&&eclCalDigitToMC1==0");
  hCellId->GetListOfFunctions()->Add(new TNamed("Description", "Cal Digit CellID for 100 MeV/c single photons in FWD endcap"));
  hCellId->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hCellId->GetXaxis()->SetTitle("CalDigit cell ID");
  hCellId->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TFile* output = TFile::Open("ECLCalDigitFWD.root", "recreate");
  hMultip->Write();
  hAmp->Write();
  hTimeFit->Write();
  hFitQuality->Write();
  hCellId->Write();
  output->Close();
  delete output;
  delete hMultip;
  delete hAmp;
  delete hTimeFit;
  delete hFitQuality;
  delete hCellId;
}


void ECLCalDigitBarrel(TTree* cd_treeBarrel)
{

  std::vector<int>* eclCalDigitToMC1=0;
  cd_treeBarrel->SetBranchAddress("eclCalDigitToMC1", &eclCalDigitToMC1);
  std::vector<int>* eclCalDigitCellId=0;
  cd_treeBarrel->SetBranchAddress("eclCalDigitCellId", &eclCalDigitCellId);

  TH1F* hMultip = new TH1F("hMultip","CalDigit Multiplicity in barrel", 30, 0., 30.);

  for(int i=0; i<cd_treeBarrel->GetEntries();i++){
    cd_treeBarrel->GetEntry(i);
    int h=0;
    for(int j=0;j<eclCalDigitToMC1->size();j++){
      if (eclCalDigitToMC1->at(j)==0)
	if (eclCalDigitCellId->at(j)<7777&&eclCalDigitCellId->at(j)>1152)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Cal Digit multiplicity for 100 MeV/c single photons in barrel"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("CalDigit multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hAmp = new TH1F("hAmp","CalDigit Amplitude in Barrel", 100, 0., 0.04);
  cd_treeBarrel->Draw("eclCalDigitAmp>>hAmp","eclCalDigitCellId<7777&&eclCalDigitCellId>1152&&eclCalDigitToMC1==0");
  hAmp->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit amplitude for 100 MeV/c single photons in Barrel"));
  hAmp->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hAmp->GetXaxis()->SetTitle("CalDigit amplitude");
  hAmp->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hTimeFit = new TH1F("hTimeFit","CalDigit TimeFit in Barrel", 110, -1100., 1100.);
  cd_treeBarrel->Draw("eclCalDigitTimeFit>>hTimeFit","eclCalDigitCellId<7777&&eclCalDigitCellId>1152&&eclCalDigitToMC1==0");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit time fit for 100 MeV/c single photons in Barrel endcap"));
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hTimeFit->GetXaxis()->SetTitle("CalDigit time fit");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hFitQuality = new TH1F("hFitQuality","CalDigit FitQuality in Barrel", 2, 0., 2.);
  hFitQuality->SetMinimum(0);
  cd_treeBarrel->Draw("eclCalDigitFitQuality>>hFitQuality","eclCalDigitCellId<7777&&eclCalDigitCellId>1152&&eclCalDigitToMC1==0");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit fit quality for 100 MeV/c single photons in Barrel endcap"));
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hFitQuality->GetXaxis()->SetTitle("CalDigit fit quality");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hCellId = new TH1F("hCellId","CalDigit CellId in Barrel", 414, 1153., 7777.);
  cd_treeBarrel->Draw("eclCalDigitCellId>>hCellId","eclCalDigitCellId<7777&&eclCalDigitCellId>1152&&eclCalDigitToMC1==0");
  hCellId->SetMinimum(0);
  hCellId->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit CellID for 100 MeV/c single photons in Barrel endcap"));
  hCellId->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hCellId->GetXaxis()->SetTitle("CalDigit cell ID");
  hCellId->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TFile* output = TFile::Open("ECLCalDigitBarrel.root", "recreate");
  hMultip->Write();
  hAmp->Write();
  hTimeFit->Write();
  hFitQuality->Write();
  hCellId->Write();
  //delete output;
  output->Close();
  delete output;
  delete hMultip;
  delete hAmp;
  delete hTimeFit;
  delete hFitQuality;
  delete hCellId;
}


void ECLCalDigitBWD(TTree* cd_treeBWD)
{

  std::vector<int>* eclCalDigitToMC1=0;
  cd_treeBWD->SetBranchAddress("eclCalDigitToMC1", &eclCalDigitToMC1);
  std::vector<int>* eclCalDigitCellId=0;
  cd_treeBWD->SetBranchAddress("eclCalDigitCellId", &eclCalDigitCellId);

  TH1F* hMultip = new TH1F("hMultip","CalDigit Multiplicity in BWD endcap", 30, 0., 30.);

  for(int i=0; i<cd_treeBWD->GetEntries();i++){
    cd_treeBWD->GetEntry(i);
    int h=0;
    for(int j=0;j<eclCalDigitToMC1->size();j++){
      if (eclCalDigitToMC1->at(j)==0)
	if (eclCalDigitCellId->at(j)>7776)
	  h++;
    }
    hMultip->Fill(h);
  }

  hMultip->GetListOfFunctions()->Add(new TNamed("Description", "Cal Digit multiplicity for 100 MeV/c single photons in BWD endcap"));
  hMultip->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip->GetXaxis()->SetTitle("CalDigit multiplicity");
  hMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hAmp = new TH1F("hAmp","CalDigit Amplitude in BWD endcap", 100, 0., 0.04);
  cd_treeBWD->Draw("eclCalDigitAmp>>hAmp","eclCalDigitCellId>7776&&eclCalDigitToMC1==0");
  hAmp->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit amplitude for 100 MeV/c single photons in BWD endcap"));
  hAmp->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hAmp->GetXaxis()->SetTitle("CalDigit amplitude");
  hAmp->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hTimeFit = new TH1F("hTimeFit","CalDigit TimeFit in BWD endcap", 110, -1100., 1100.);
  cd_treeBWD->Draw("eclCalDigitTimeFit>>hTimeFit","eclCalDigitCellId>7776&&eclCalDigitToMC1==0");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit time fit for 100 MeV/c single photons in BWD endcap"));
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hTimeFit->GetXaxis()->SetTitle("CalDigit time fit");
  hTimeFit->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hFitQuality = new TH1F("hFitQuality","CalDigit FitQuality in BWD endcap", 2, 0., 2.);
  hFitQuality->SetMinimum(0);
  cd_treeBWD->Draw("eclCalDigitFitQuality>>hFitQuality","eclCalDigitCellId>7776&&eclCalDigitToMC1==0");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit fit quality for 100 MeV/c single photons in BWD endcap"));
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hFitQuality->GetXaxis()->SetTitle("CalDigit fit quality");
  hFitQuality->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  TH1F* hCellId = new TH1F("hCellId","CalDigit CellId in BWD endcap", 240, 7777., 8737.);
  hCellId->SetMinimum(0);
  cd_treeBWD->Draw("eclCalDigitCellId>>hCellId","eclCalDigitCellId>7776&&eclCalDigitToMC1==0");
  hCellId->GetListOfFunctions()->Add(new TNamed("Description", "CalDigit cell ID for 100 MeV/c single photons in BWD endcap"));
  hCellId->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hCellId->GetXaxis()->SetTitle("CalDigit cell ID");
  hCellId->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 


  TFile* output = TFile::Open("ECLCalDigitBWD.root", "recreate");
  hMultip->Write();
  hAmp->Write();
  hTimeFit->Write();
  hFitQuality->Write();
  hCellId->Write();
  output->Close();
  delete output;
  delete hMultip;
  delete hAmp;
  delete hTimeFit;
  delete hFitQuality;
  delete hCellId;
}


void ECLBkg(TTree* bkg_tree)
{

  TFile* output = TFile::Open("ECLBkg.root", "recreate");

  TH1F* bkgClusterE = new TH1F("bkgClusterE", "Cluster energy, bkg only", 60, 0., 0.2);
  bkg_tree->Draw("eclClusterEnergy>>bkgClusterE","eclClusterEnergy>0&&eclClusterHypothesisId==5");
  bkgClusterE->GetXaxis()->SetTitle("Cluster energy (GeV)");
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for bkg clusters")); 
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be peaked at 20. (threshold value)"));
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  bkgClusterE->Write();

  TH1F* bkgClusterTheta = new TH1F("bkgClusterTheta", "Cluster theta, bkg only", 50, 0, 3.2);
  bkg_tree->Draw("eclClusterTheta>>bkgClusterTheta","eclClusterEnergy>0&&eclClusterHypothesisId==5");
  bkgClusterTheta->GetXaxis()->SetTitle("#theta (rad)");
  bkgClusterTheta->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster theta for bkg clusters")); 
  bkgClusterTheta->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape."));
  bkgClusterTheta->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  bkgClusterTheta->Write();

  TH1F* bkgClusterPhi = new TH1F("bkgClusterPhi", "Cluster phi, bkg only", 50, -3.6, 3.6);
  bkg_tree->Draw("eclClusterPhi>>bkgClusterPhi","eclClusterEnergy>0&&eclClusterHypothesisId==5");
  bkgClusterPhi->GetXaxis()->SetTitle("#phi (rad)");
  bkgClusterPhi->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster phi for bkg clusters")); 
  bkgClusterPhi->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape."));
  bkgClusterPhi->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 
  bkgClusterPhi->Write();

  std::vector<int>* eclClusterHypothesisId=0;
  bkg_tree->SetBranchAddress("eclClusterHypothesisId", &eclClusterHypothesisId);

  TH1F* bkgClusterMultip = new TH1F("bkgClusterMultip","Cluster Multiplicity", 150, 0., 150.);

  for(int i=0; i<bkg_tree->GetEntries();i++){
    bkg_tree->GetEntry(i);
    int h=0;
    for(int j=0;j<eclClusterHypothesisId->size();j++){
      if (eclClusterHypothesisId->at(j)==5)
	  h++;
    }
    bkgClusterMultip->Fill(h);
  }

  bkgClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity");
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Description","ECL cluster multiplicity for bkg")); 
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Check","Cluster multiplicity should be around 55 (Feb 2018)"));
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgClusterMultip->Write();

  TH1F* bkgOutOfTimeDigitsFWD = new TH1F("bkgOutOfTimeDigitsFWD","# of out-of-time digits in FWD endcap", 100, 0., 100.);

  bkgOutOfTimeDigitsFWD->GetXaxis()->SetTitle("# of out-of-time digits in FWD endcap");
  bkg_tree->Draw("eclNumOutOfTimeDigitsFwd>>bkgOutOfTimeDigitsFWD");
  bkgOutOfTimeDigitsFWD->GetListOfFunctions()->Add(new TNamed("Description","# of out-of-time digits in FWD endcap")); 
  bkgOutOfTimeDigitsFWD->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 50 (Feb 2018)"));
  bkgOutOfTimeDigitsFWD->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgOutOfTimeDigitsFWD->Write();

 TH1F* bkgOutOfTimeDigitsBRL = new TH1F("bkgOutOfTimeDigitsBRL","# of out-of-time digits in barrel", 150, 200., 500.);

  bkgOutOfTimeDigitsBRL->GetXaxis()->SetTitle("# of out-of-time digits in barrel");
  bkg_tree->Draw("eclNumOutOfTimeDigitsBrl>>bkgOutOfTimeDigitsBRL");
  bkgOutOfTimeDigitsBRL->GetListOfFunctions()->Add(new TNamed("Description","# of out-of-time digits in barrel")); 
  bkgOutOfTimeDigitsBRL->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 340 (Feb 2018)"));
  bkgOutOfTimeDigitsBRL->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgOutOfTimeDigitsBRL->Write();

  TH1F* bkgOutOfTimeDigitsBWD = new TH1F("bkgOutOfTimeDigitsBWD","# of out-of-time digits in BWD endcap", 120, 60., 180.);

  bkgOutOfTimeDigitsBWD->GetXaxis()->SetTitle("# of out-of-time digits in BWD endcap");
  bkg_tree->Draw("eclNumOutOfTimeDigitsBwd>>bkgOutOfTimeDigitsBWD");
  bkgOutOfTimeDigitsBWD->GetListOfFunctions()->Add(new TNamed("Description","# of out-of-time digits in BWD endcap")); 
  bkgOutOfTimeDigitsBWD->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 100 (Feb 2018)"));
  bkgOutOfTimeDigitsBWD->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgOutOfTimeDigitsBWD->Write();



  TH1F* bkgRejectedShowersFWD = new TH1F("bkgRejectedShowersFWD","# of rejected showers in FWD endcap", 100, 0., 100.);

  bkgRejectedShowersFWD->GetXaxis()->SetTitle("# of rejected ECL showers in FWD endcap");
  bkg_tree->Draw("eclNumRejectedShowersFwd>>bkgRejectedShowersFWD");
  bkgRejectedShowersFWD->GetListOfFunctions()->Add(new TNamed("Description","# of rejected ECL showers in FWD endcap")); 
  bkgRejectedShowersFWD->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 31 (Feb 2018)"));
  bkgRejectedShowersFWD->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgRejectedShowersFWD->Write();

  TH1F* bkgRejectedShowersBRL = new TH1F("bkgRejectedShowersBRL","# of rejected showers in barrel", 150, 100., 250.);

  bkgRejectedShowersBRL->GetXaxis()->SetTitle("# of rejected ECL showers in barrel");
  bkg_tree->Draw("eclNumRejectedShowersBrl>>bkgRejectedShowersBRL");
  bkgRejectedShowersBRL->GetListOfFunctions()->Add(new TNamed("Description","# of rejected ECL showers in barrel")); 
  bkgRejectedShowersBRL->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 180 (Feb 2018)"));
  bkgRejectedShowersBRL->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgRejectedShowersBRL->Write();

  TH1F* bkgRejectedShowersBWD = new TH1F("bkgRejectedShowersBWD","# of rejected showers in BWD endcap", 100, 0., 100.);

  bkgRejectedShowersBWD->GetXaxis()->SetTitle("# of rejected ECL showers in BWD endcap");
  bkg_tree->Draw("eclNumRejectedShowersBwd>>bkgRejectedShowersBWD");
  bkgRejectedShowersBWD->GetListOfFunctions()->Add(new TNamed("Description","# of rejected ECL showers in BWD endcap")); 
  bkgRejectedShowersBWD->GetListOfFunctions()->Add(new TNamed("Check","Multiplicity should be around 43 (Feb 2018)"));
  bkgRejectedShowersBWD->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it"));
  bkgRejectedShowersBWD->Write();
 



  output->Close();
  delete output;
}

void ECL2D(TTree* bkg_tree)
{

  TFile* output = TFile::Open("ECL2D.root", "recreate");

  gStyle->SetOptStat(000000);

  TH2F* BDyz = new TH2F("BDyz", "Cluster position (yz-plane)", 200, -1.5, 2.5, 100, -1.5, 1.5);
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

  bkg_tree->Draw("(2.402*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(2.402*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz7","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)");
  BDyz7->Draw("colz");

  BDyz7->GetXaxis()->SetTitle("x (m)");
  BDyz7->GetYaxis()->SetTitle("y (m)");
  BDyz7->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz7->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in the FWD ecl")); 
  BDyz7->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz7->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz7->Write();

  bkg_tree->Draw("(1.669*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(1.669*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz8","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)");
  BDyz8->Draw("colz");

  BDyz8->GetXaxis()->SetTitle("x (m)");
  BDyz8->GetYaxis()->SetTitle("y (m)");
  BDyz8->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz8->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in the BWD ecl")); 
  BDyz8->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz8->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz8->Write();

  TH2F* BDyz9 = new TH2F("BDyz9", "Cluster position barrel (xy-plane)", 200, -2, 2, 200, -1.5, 1.5);
  bkg_tree->Draw("(1.250*TMath::Sin(eclClusterPhi)):(1.250*TMath::Cos(eclClusterPhi))>>BDyz9","(32.20*(3.1415/180))<eclClusterTheta<(180-51.28)*(3.1415/180)");
  BDyz9->Draw("colz");
  BDyz9->GetXaxis()->SetTitle("x (m)");
  BDyz9->GetYaxis()->SetTitle("y (m)");
  BDyz9->GetListOfFunctions()->Add(new TNamed("MetaOptions","colz")); 
  BDyz9->GetListOfFunctions()->Add(new TNamed("Description","Bkg cluster position in barrel ecl")); 
  BDyz9->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz9->GetListOfFunctions()->Add(new TNamed("Contact","elisa.manoni@pg.infn.it")); 

  BDyz9->Write();
  
  output->Close();
  delete output;
}

Double_t Novosibirsk(Double_t *xp,Double_t *par) {

  Double_t x=xp[0];
  Double_t a=par[0];
  Double_t mu=par[1];
  Double_t sigma=par[2];
  Double_t tau=par[3];
  
  if (TMath::Abs(tau) < 1.e-7) {
    return TMath::Exp( -0.5 * TMath::Power( ( (x - mu) / sigma), 2 ));
  }
 
  Double_t arg = 1.0 - ( x - mu ) * tau / sigma;
 
  if (arg < 1.e-7) {
    return 0.0;
  }
   
  Double_t log = TMath::Log(arg);   
  static const Double_t xi = 2.3548200450309494; // 2 Sqrt( Ln(4) )
  Double_t width_zero = ( 2.0 / xi ) * TMath::ASinH( tau * xi * 0.5 );
  Double_t width_zero2 = width_zero * width_zero;
  Double_t exponent = ( -0.5 / (width_zero2) * log * log ) - ( width_zero2 * 0.5 );
   
  return a*TMath::Exp(exponent);   
}
