/* ECLPion.C 
 * ROOT macro for ECL validation plots 
 * Authors: Poyuan Chen, Benjamin Oberhof 
 * 2014
*/

/*
<header>
<input>ECLMuonOutput.root</input>
<output>ECLMuon.root</output>
<contact>ecl2ml@bpost.kek.jp</contact>
</header>
*/


void ECLMuon()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../ECLMuonOutput.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLMuon.root", "recreate");

  // an example of a 1D histogram
  TH1F* hMuonsE = new TH1F("hMuonsE", "Reconstructed Shower Energy for 1GeV Muon, the typical energy is 0.2 GeV", 100, 0., 1.2);
  hMuonsE->GetXaxis()->SetTitle("Shower Energy (GeV)");
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Description","Energy release in the ECL for 1 GeV muons")); 
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 200 MeV"));
  hMuonsE->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("ECLShowers.m_Energy>>hMuonsE");
  hMuonsE->Write();

  // an example of a 1D histogram
  TH1F* hMuonsFake = new TH1F("hMuonsFake", "Fake Reconstructed camma for 1000 muons should be less than 5%", 10,0,10);
  hMuonsFake->SetMaximum(6);
  hMuonsFake->GetXaxis()->SetTitle("Shower Id");
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Description","Fake reconstructed gamma from 1000 muons")); 
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Check","Should be low"));
  hMuonsFake->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("ECLGammas.m_ShowerId>>hMuonsFake");
  hMuonsFake->Write();



  delete output;
}
