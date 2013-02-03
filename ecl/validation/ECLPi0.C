void ECLPi0()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../ECLPi0Output.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLPi0.root", "recreate");

  // an example of a 1D histogram
  TH1F* hPi0s = new TH1F("hPi0s", "ECL Reconstructed Pi0 Energy for 1 GeV/c Pi0", 100, 0.2, 1.2);
  hPi0s->GetXaxis()->SetTitle("Pi0 Energy (GeV)");
  tree->Draw("ECLPi0s.m_Energy>>hPi0s");
  hPi0s->Write();

  // an example of a 1D histogram
  TH1F* hPi0mass = new TH1F("hPi0mass", "ECL combined Pi0 Mass for 1 GeV/c Pi0", 100, 0.08, 0.18);
  hPi0mass->GetXaxis()->SetTitle("Pi0 combined mass (GeV/c^{2})");
  tree->Draw("ECLPi0s.m_Mass>>hPi0mass");
  hPi0mass->Write();

  // an example of a 1D histogram
  TH1F* hPi0massFit = new TH1F("hPi0massFit", "ECL constrainted  Pi0 Mass for 1 GeV/c Pi0", 100, 0.08, 0.18);
  hPi0massFit->GetXaxis()->SetTitle("Pi0 constrainted  mass (GeV/c^{2})");
  tree->Draw("ECLPi0s.m_MassFit>>hPi0massFit");
  hPi0massFit->Write();



  delete output;
}
