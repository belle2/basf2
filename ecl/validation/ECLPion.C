void ECLPion()
{

  TFile* inputFile = TFile::Open("../ECLPionOutput.root");
  TTree* tree = (TTree*) inputFile->Get("m_tree");

  TH1F* hClusterE = new TH1F("PiClusterE", "Cluster energy, 1 GeV charged pion", 100, 0., 1.2);
  hClusterE->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hClusterE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for single 1GeV charged pion")); 
  hClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be 0.2 GeV"));
  hClusterE->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  //hClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be 0.2 GeV"));
  m_tree->Draw("eclClusterEnergy>>hClusterE","eclShowerEnergy>0");

  TH1F* hClusterMultip = new TH1F("PiClusterMultip", "Cluster multiplicity, 1GeV charged pion", 10,0,10);
  hClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity 1 GeV charged pion");
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Description",
    "ECL cluster multiplicity for 1 GeV charged pion")); 
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Check",
    "Cluster multiplicity should be low"));
  m_tree->Draw("eclClusterMultip>>hClusterMultip","eclClusterMultip>0");

  // write output file for the validation histograms
  TFile* output = TFile::Open("ECLPion.root", "recreate");
  hClusterE->Write();
  hClusterMultip->Write();
  
  delete output;
}
