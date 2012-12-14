void ECLPi0()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../ECLPi0Output.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLPi0.root", "recreate");

  // an example of a 1D histogram
  TH1F* hPi0s = new TH1F("hPi0s", "ECL Reconstructed Pi0 Energy for 1GeV Pi0", 100, 0.2, 1.2);
  hPi0s->GetXaxis()->SetTitle("Pi0 Energy (GeV)");
  tree->Draw("ECLPi0s.m_Energy>>hPi0s");
  hPi0s->Write();


  delete output;
}
