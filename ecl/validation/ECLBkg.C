/* ECLBkg.C 
 * ROOT macro for ECL validation plots 
 * Author: Benjamin Oberhof 
 * 2014
*/

void ECLBkg()
{

  TFile* inputFile = TFile::Open("../ECLBkgOutput.root");
  TTree* tree = (TTree*) inputFile->Get("m_tree");

  TFile* output = TFile::Open("ECLBkg.root", "recreate");

  TH1F* bkgClusterE = new TH1F("bkgClusterE", "Cluster energy, bkg only", 100, 0., 0.5);
  bkgClusterE->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Description","Reconstructed cluster energy for bkg clusters")); 
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Check","Typical energy should be peaked at 0"));
  bkgClusterE->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("eclClusterEnergy>>bkgClusterE","eclClusterEnergy>0");
  bkgClusterE->Write();

  TH1F* bkgClusterMultip = new TH1F("bkgClusterMultip", "Cluster multiplicity bkg only", 150, 0, 300);
  bkgClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity Bkg");
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Description","ECL cluster multiplicity for bkg")); 
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Check","Cluster multiplicity should be around 160 (Jan 2015)"));
  bkgClusterMultip->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterMultip>>bkgClusterMultip","eclClusterMultip>0");
  bkgClusterMultip->Write();
 
  output->Close();
  delete output;
}
