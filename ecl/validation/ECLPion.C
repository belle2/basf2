/* ECLPion.C 
 * ROOT macro for ECL validation plots 
 * Author Benjamin Oberhof
*/

void ECLPion(){

  TFile* inputFile = TFile::Open("../ECLPionOutput.root");
  TTree* tree = (TTree*) inputFile->Get("m_tree");

  // write output file for the validation histograms
  TFile* output = TFile::Open("ECLPion.root", "recreate");

  TH1F* hClusterE = new TH1F("hClusterE", "Cluster energy, 1 GeV charged pion", 100, 0., 1.2);
  hClusterE->GetXaxis()->SetTitle("Cluster energy (GeV)");
  hClusterE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for single 1GeV charged pion")); 
  hClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be 0.2 GeV"));
  hClusterE->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("eclClusterEnergy>>hClusterE","eclClusterEnergy>0");
  hClusterE->Write();


  TH1F* hClusterMultip = new TH1F("hClusterMultip", "Cluster multiplicity, 1GeV charged pion", 10,0,10);
  hClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity");
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Description","ECL cluster multiplicity for 1 GeV charged pion")); 
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Check","Cluster multiplicity should be low"));
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("eclClusterMultip>>hClusterMultip","eclClusterMultip>0");
  hClusterMultip->Write();
  
  output->Close();
  delete output;
}
