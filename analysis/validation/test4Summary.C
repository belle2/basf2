void test4Summary()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../MDSTtoUDST.root");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("MDSTSummary.root", "recreate");

  TH1I* h_nmcparticles = (TH1I*) input->Get("h_nMCParticles");
  double nmcparticles=h_nmcparticles->GetMean();

  TH1I* h_nmcphotons = (TH1I*) input->Get("h_nmcphotons");
  double nmcphotons=h_nmcphotons->GetMean();

  TH1I* h_nTracks = (TH1I*) input->Get("h_nTracks");
  double nTracks=h_nTracks->GetMean();

  TH1I* h_nGFTracks = (TH1I*) input->Get("h_nGFTracks");
  double nGFTracks=h_nGFTracks->GetMean();

  TH1I* h_nECLGammas = (TH1I*) input->Get("h_nECLGammas");
  double nECLGammas=h_nECLGammas->GetMean();

  TH1I* h_nECLShowers = (TH1I*) input->Get("h_nECLGammas");
  double nECLShowers=h_nECLShowers->GetMean();


  TH1F* hCands = new TH1F("hCands", "Cands", 20, 0, 20);
  hCands->GetXaxis()->SetTitle("Container");
  hCands->GetYaxis()->SetTitle("Mean entries");

  hCands->SetBinContent(1,nmcparticles);
  hCands->GetXaxis()->SetBinLabel(1,"MCParticles");

  hCands->SetBinContent(2,nmcphotons);
  hCands->GetXaxis()->SetBinLabel(2,"MCParticles(Photons)");

  hCands->SetBinContent(3,nTracks);
  hCands->GetXaxis()->SetBinLabel(3,"Tracks");

  hCands->SetBinContent(4,nGFTracks);
  hCands->GetXaxis()->SetBinLabel(4,"GFTracks");

  hCands->SetBinContent(5,nECLGammas);
  hCands->GetXaxis()->SetBinLabel(5,"ECLGammas");

  hCands->SetBinContent(6,nECLShowers);
  hCands->GetXaxis()->SetBinLabel(6,"ECLShowers");

  output->Write();

  delete output;
}
