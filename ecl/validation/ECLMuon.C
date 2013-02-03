void ECLMuon()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../ECLMuonOutput.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLMuon.root", "recreate");

  // an example of a 1D histogram
  TH1F* hShowers = new TH1F("hShowers", "Reconstructed Shower Energy for 1GeV Muon, the typical energy is 0.2 GeV", 100, 0., 1.2);
  hShowers->GetXaxis()->SetTitle("Shower Energy (GeV)");
  tree->Draw("ECLShowers.m_Energy>>hShowers");
  hShowers->Write();

  // an example of a 1D histogram
  TH1F* hGammas = new TH1F("hGammas", "Fake Reconstructed Gamma from  1000 muons should be less than 5%", 10,0,10);
  hGammas->GetXaxis()->SetTitle("Shower Id");
  tree->Draw("ECLGammas.m_ShowerId>>hGammas");
  hGammas->Write();



  delete output;
}
