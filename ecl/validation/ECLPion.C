void ECLPion()
{

  TFile* inputFile = TFile::Open("../ECLPionOutput.root");
  TTree* tree = (TTree*) inputFile->Get("m_tree");

  //  TH1F* hShowers = new TH1F("hShowers", "Reconstructed Shower Energy for 1GeV Muon, the typical energy is 0.2 GeV", 100, 0., 1.2);
  TH1F* hClusterE = new TH1F("hClusterE", "ECL shower energy, charged pions", 100, 0., 1.2);
  hClusterE->GetXaxis()->SetTitle("Shower Energy (GeV)");
  hClusterE->GetListOfFunctions()->Add(new TNamed("Description",
    "Reconstructed shower energy for single 1GeV charged pion")); 
  hClusterE->GetListOfFunctions()->Add(new TNamed("Check",
    "Typical energy should be 0.2 GeV"));
  m_tree->Draw("eclShowerEnergy>>hClusterE","eclShowerEnergy>0");
  //hShowers->Write();

  //  TH1F* hGammas = new TH1F("hGammas", "Fake Reconstructed Gamma from  1000 muons should be less than 5%", 10,0,10);
  TH1F* hClusterMultip = new TH1F("hClusterMultip", "Cluster Multiplicity Pions", 10,0,10);
  hClusterMultip->GetXaxis()->SetTitle("ECL cluster multiplicity charged pion");
  hClusterE->GetListOfFunctions()->Add(new TNamed("Description",
    "ECL cluster multiplicity for 1 GeV charged pion")); 
  hClusterE->GetListOfFunctions()->Add(new TNamed("Check",
    "Cluster multiplicity should be low"));
  m_tree->Draw("eclClusterMultip>>hClusterMultip","eclClusterMultip>0");
  //hGammas->Write();

  //TH1F* hE9oE25_500 = new TH1F("hE9oE25_500","E9/E25", 100, 0., 1.05);
  //hE9oE25_500->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 500 MeV/c single photons"));

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLPion.root", "recreate");
  hClusterE->Write();
  hClusterMultip->Write();
  
  delete output;
}
