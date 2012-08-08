void example()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../EvtGenSimRec.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("example.root", "recreate");

  // an example of a 1D histogram
  TH1F* hTopLHs = new TH1F("hTopLHs", "TOP Likelihoods", 100, -200, 200);
  hTopLHs->GetXaxis()->SetTitle("TOP LL(#pi)-LL(K)");
  hTopLHs->GetYaxis()->SetTitle("tracks");
  tree->Draw("TOPLikelihoodss.m_logL_pi-TOPLikelihoodss.m_logL_K>>hTopLHs");
  hTopLHs->Write();

  // an example of a 2D histogram
  TH2F* hMCVertex = new TH2F("hMCVertex", "Vertex of MC Particles", 100, -400, 400, 100, -400, 400);
  hMCVertex->GetXaxis()->SetTitle("x [cm]");
  hMCVertex->GetYaxis()->SetTitle("y [cm]");
  tree->Draw("MCParticles.m_productionVertex_y:MCParticles.m_productionVertex_x>>hMCVertex");
  hMCVertex->Write();

  delete output;
}
