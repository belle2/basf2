void ECLMuon()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../ECLMuonOutput.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLMuon.root", "recreate");

  // an example of a 1D histogram
  TH1F* hShowers = new TH1F("hShowers", "ECL Reconstructed Shower Energy for 1000 single 1GeV Muon, typical energy is 0.2GeV ", 100, 0., 1.2);
  hShowers->GetXaxis()->SetTitle("Shower Energy (GeV)");
  tree->Draw("ECLShowers.m_Energy>>hShowers");
  hShowers->Write();

  // an example of a 1D histogram
  TH1F* hGammas = new TH1F("hGammas", "Fake Reconstructed Gamma is extected about 1% for 1000 Muon", 10,0,10);
  hGammas->GetXaxis()->SetTitle("Shower Id");
  tree->Draw("ECLGammas.m_ShowerId>>hGammas");
  hGammas->Write();



  delete output;
}
