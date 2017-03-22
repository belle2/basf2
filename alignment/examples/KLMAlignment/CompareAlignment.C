
/**
 * Comparison of initial displacements and alignment result.
 */
void CompareAlignment(const char *displacementFile, const char *alignmentFile,
                      const char *comparisonFile)
{
  int i, n;
  int endcap, layer, sector, plane, segment, param;
  float value0, value, error;
  float val0[2][14][4][2][5][2];
  TFile *fDisplacement = new TFile(displacementFile);
  TTree *tDisplacement = (TTree*)fDisplacement->Get("eklm");
  TFile *fAlignment = new TFile(alignmentFile);
  TTree *tAlignment = (TTree*)fAlignment->Get("eklm");
  TFile *fComparison = new TFile(comparisonFile, "recreate");
  TTree *tComparison = new TTree("eklm", "");
  tDisplacement->SetBranchAddress("endcap", &endcap);
  tDisplacement->SetBranchAddress("layer", &layer);
  tDisplacement->SetBranchAddress("sector", &sector);
  tDisplacement->SetBranchAddress("plane", &plane);
  tDisplacement->SetBranchAddress("segment", &segment);
  tDisplacement->SetBranchAddress("param", &param);
  tDisplacement->SetBranchAddress("value", &value);
  tAlignment->SetBranchAddress("endcap", &endcap);
  tAlignment->SetBranchAddress("layer", &layer);
  tAlignment->SetBranchAddress("sector", &sector);
  tAlignment->SetBranchAddress("plane", &plane);
  tAlignment->SetBranchAddress("segment", &segment);
  tAlignment->SetBranchAddress("param", &param);
  tAlignment->SetBranchAddress("value", &value);
  tAlignment->SetBranchAddress("error", &error);
  tComparison->Branch("endcap", &endcap, "endcap/I");
  tComparison->Branch("layer", &layer, "layer/I");
  tComparison->Branch("sector", &sector, "sector/I");
  tComparison->Branch("plane", &plane, "plane/I");
  tComparison->Branch("segment", &segment, "segment/I");
  tComparison->Branch("param", &param, "param/I");
  tComparison->Branch("value0", &value0, "value0/F");
  tComparison->Branch("value", &value, "value/F");
  tComparison->Branch("error", &error, "error/F");
  n = tDisplacement->GetEntries();
  for (i = 0; i < n; i++) {
    tDisplacement->GetEntry(i);
    val0[endcap - 1][layer - 1][sector - 1][plane - 1][segment - 1][param - 1] =
      value;
  }
  n = tAlignment->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignment->GetEntry(i);
    value0 = val0[endcap - 1][layer - 1][sector - 1][plane - 1][segment - 1]
                 [param - 1];
    tComparison->Fill();
  }
  fComparison->cd();
  tComparison->Write();
  delete tComparison;
  delete fComparison;
  delete tAlignment;
  delete fAlignment;
  delete tDisplacement;
  delete fDisplacement;
}
