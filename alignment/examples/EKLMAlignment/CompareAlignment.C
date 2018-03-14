
/**
 * Get parameter index.
 */
int parameterIndex(int param)
{
  switch (param) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 6:
      return 2;
  }
  return -1;
}

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
  TTree *tDisplacementSector = (TTree*)fDisplacement->Get("eklm_sector");
  TTree *tDisplacementSegment = (TTree*)fDisplacement->Get("eklm_segment");
  TFile *fAlignment = new TFile(alignmentFile);
  TTree *tAlignmentSector = (TTree*)fAlignment->Get("eklm_sector");
  TTree *tAlignmentSegment = (TTree*)fAlignment->Get("eklm_segment");
  TFile *fComparison = new TFile(comparisonFile, "recreate");
  TTree *tComparisonSector = new TTree("eklm_sector", "");
  TTree *tComparisonSegment = new TTree("eklm_segment", "");
  /* Sector. */
  tDisplacementSector->SetBranchAddress("endcap", &endcap);
  tDisplacementSector->SetBranchAddress("layer", &layer);
  tDisplacementSector->SetBranchAddress("sector", &sector);
  tDisplacementSector->SetBranchAddress("param", &param);
  tDisplacementSector->SetBranchAddress("value", &value);
  tAlignmentSector->SetBranchAddress("endcap", &endcap);
  tAlignmentSector->SetBranchAddress("layer", &layer);
  tAlignmentSector->SetBranchAddress("sector", &sector);
  tAlignmentSector->SetBranchAddress("param", &param);
  tAlignmentSector->SetBranchAddress("value", &value);
  tAlignmentSector->SetBranchAddress("error", &error);
  tComparisonSector->Branch("endcap", &endcap, "endcap/I");
  tComparisonSector->Branch("layer", &layer, "layer/I");
  tComparisonSector->Branch("sector", &sector, "sector/I");
  tComparisonSector->Branch("param", &param, "param/I");
  tComparisonSector->Branch("value0", &value0, "value0/F");
  tComparisonSector->Branch("value", &value, "value/F");
  tComparisonSector->Branch("error", &error, "error/F");
  n = tDisplacementSector->GetEntries();
  for (i = 0; i < n; i++) {
    /*
     * Usage of (param - 1) is intentional: EKLMAlignment module uses
     * number 3 for dalpha.
     */
    tDisplacementSector->GetEntry(i);
    val0[endcap - 1][layer - 1][sector - 1][0][0][param - 1] =
      value;
  }
  n = tAlignmentSector->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignmentSector->GetEntry(i);
    value0 =
      val0[endcap - 1][layer - 1][sector - 1][0][0][parameterIndex(param)];
    tComparisonSector->Fill();
  }
  /* Segment. */
  tDisplacementSegment->SetBranchAddress("endcap", &endcap);
  tDisplacementSegment->SetBranchAddress("layer", &layer);
  tDisplacementSegment->SetBranchAddress("sector", &sector);
  tDisplacementSegment->SetBranchAddress("plane", &plane);
  tDisplacementSegment->SetBranchAddress("segment", &segment);
  tDisplacementSegment->SetBranchAddress("param", &param);
  tDisplacementSegment->SetBranchAddress("value", &value);
  tAlignmentSegment->SetBranchAddress("endcap", &endcap);
  tAlignmentSegment->SetBranchAddress("layer", &layer);
  tAlignmentSegment->SetBranchAddress("sector", &sector);
  tAlignmentSegment->SetBranchAddress("plane", &plane);
  tAlignmentSegment->SetBranchAddress("segment", &segment);
  tAlignmentSegment->SetBranchAddress("param", &param);
  tAlignmentSegment->SetBranchAddress("value", &value);
  tAlignmentSegment->SetBranchAddress("error", &error);
  tComparisonSegment->Branch("endcap", &endcap, "endcap/I");
  tComparisonSegment->Branch("layer", &layer, "layer/I");
  tComparisonSegment->Branch("sector", &sector, "sector/I");
  tComparisonSegment->Branch("plane", &plane, "plane/I");
  tComparisonSegment->Branch("segment", &segment, "segment/I");
  tComparisonSegment->Branch("param", &param, "param/I");
  tComparisonSegment->Branch("value0", &value0, "value0/F");
  tComparisonSegment->Branch("value", &value, "value/F");
  tComparisonSegment->Branch("error", &error, "error/F");
  n = tDisplacementSegment->GetEntries();
  for (i = 0; i < n; i++) {
    tDisplacementSegment->GetEntry(i);
    val0[endcap - 1][layer - 1][sector - 1][plane - 1][segment - 1][param - 1] =
      value;
  }
  n = tAlignmentSegment->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignmentSegment->GetEntry(i);
    value0 = val0[endcap - 1][layer - 1][sector - 1][plane - 1][segment - 1]
                 [param - 1];
    tComparisonSegment->Fill();
  }
  fComparison->cd();
  tComparisonSector->Write();
  tComparisonSegment->Write();
  delete tComparisonSegment;
  delete tComparisonSector;
  delete fComparison;
  delete tAlignmentSegment;
  delete tAlignmentSector;
  delete fAlignment;
  delete tDisplacementSegment;
  delete tDisplacementSector;
  delete fDisplacement;
}
