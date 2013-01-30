void EKLMMuon()
{
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));  
  TFile* fin = new TFile("../EKLMMuonOutput.root");
  TTree* tree = (TTree*) fin->Get("tree");
  TFile* fout = new TFile("EKLMMuon.root", "recreate");
  TH1F* h2dtres = new TH1F("muon_h2dtres", "EKLM muon 2d hits time resolution",
    100, -10, 10);
  h2dtres->SetXTitle("ns");
  h2dtres->SetYTitle("Events");
  tree->Draw("EKLMHit2ds.m_Time-EKLMHit2ds.m_MCTime>>muon_h2dtres");
  h2dtres->Write();
  delete h2dtres;
  delete fin;
  delete fout;
}

