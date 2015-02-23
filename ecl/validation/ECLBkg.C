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

  TH1F* hClusterE = new TH1F("hClusterE", "ECL cluster energy, bkg only", 100, 0., 0.5);
  hClusterE->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hClusterE->GetListOfFunctions()->Add(new TNamed("Description",
    "Reconstructed cluster energy for bkg clusters")); 
  hClusterE->GetListOfFunctions()->Add(new TNamed("Check",
    "Typical energy should be peaked at 0"));
  hClusterE->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("eclClusterEnergy>>hClusterE","eclClusterEnergy>0");
  hClusterE->Write();

  TH1F* hClusterMultip = new TH1F("hClusterMultip", "Cluster Multiplicity Bkg Only", 150, 0, 300);
  hClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity Bkg");
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Description",
    "ECL cluster multiplicity for bkg")); 
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Check",
    "Cluster multiplicity should be around 160 (Jan 2015)"));
  hClusterMultip->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterMultip>>hClusterMultip","eclClusterMultip>0");
  hClusterMultip->Write();
  
  output->Close();
  delete output;
}
