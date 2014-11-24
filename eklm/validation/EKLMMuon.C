/*
<header>
<input>EKLMMuonOutput.root</input>
<output>EKLMMuon.root</output>
<contact>Timofey Uglov, uglov@itep.ru</contact>
</header>
*/





void EKLMMuon()
{
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));  
  TFile* fin = new TFile("../EKLMMuonOutput.root");
  TTree* tree = (TTree*) fin->Get("tree");
  TFile* fout = new TFile("EKLMMuon.root", "recreate");
  TH1F* h1dtres = new TH1F("muon_h1dtres", "EKLM muon 1d hits time resolution",
    100, -10, 10);
  TH1F* h2dtres = new TH1F("muon_h2dtres", "EKLM muon 2d hits time resolution",
    100, -10, 10);
  h1dtres->SetXTitle("ns");
  h1dtres->SetYTitle("Events");
  tree->Draw("EKLMDigits.m_Time-EKLMDigits.m_sMCTime>>muon_h1dtres", "EKLMDigits.m_good==1");
  h1dtres->GetListOfFunctions()->Add(new TNamed("Description",
    "Time resolution")); 
  h1dtres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias. See bug #997.")); 
  h1dtres->Write();
  h2dtres->SetXTitle("ns");
  h2dtres->SetYTitle("Events");
  tree->Draw("EKLMHit2ds.m_Time-EKLMHit2ds.m_MCTime>>muon_h2dtres");
  h2dtres->GetListOfFunctions()->Add(new TNamed("Description",
    "Time resolution")); 
  h2dtres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias. See bug #997.")); 
  h2dtres->Write();
  delete h2dtres;
  delete fin;
  delete fout;
}

