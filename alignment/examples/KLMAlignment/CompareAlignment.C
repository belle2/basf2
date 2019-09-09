
/**
 * Comparison of initial displacements and alignment result.
 */
void CompareAlignment(const char *displacementFile, const char *alignmentFile,
                      const char *comparisonFile)
{
  int i, n;
  int section, layer, sector, plane, segment, param;
  float value0, value, error;
  float val0[2][14][4][2][5][6];
  TFile *fDisplacement;
  TTree *tDisplacementEKLMModule, *tDisplacementEKLMSegment;
  if (displacementFile != nullptr) {
    fDisplacement = new TFile(displacementFile);
    tDisplacementEKLMModule = (TTree*)fDisplacement->Get("eklm_module");
    tDisplacementEKLMSegment = (TTree*)fDisplacement->Get("eklm_segment");
  }
  TFile *fAlignment = new TFile(alignmentFile);
  TTree *tAlignmentEKLMModule = (TTree*)fAlignment->Get("eklm_module");
  TTree *tAlignmentEKLMSegment = (TTree*)fAlignment->Get("eklm_segment");
  TTree *tAlignmentBKLMModule = (TTree*)fAlignment->Get("bklm_module");
  TFile *fComparison = new TFile(comparisonFile, "recreate");
  TTree *tComparisonEKLMModule = new TTree("eklm_module", "");
  TTree *tComparisonEKLMSegment = new TTree("eklm_segment", "");
  TTree *tComparisonBKLMModule = new TTree("bklm_module", "");
  /* Sector. */
  if (displacementFile != nullptr) {
    tDisplacementEKLMModule->SetBranchAddress("section", &section);
    tDisplacementEKLMModule->SetBranchAddress("layer", &layer);
    tDisplacementEKLMModule->SetBranchAddress("sector", &sector);
    tDisplacementEKLMModule->SetBranchAddress("param", &param);
    tDisplacementEKLMModule->SetBranchAddress("value", &value);
  }
  tAlignmentEKLMModule->SetBranchAddress("section", &section);
  tAlignmentEKLMModule->SetBranchAddress("layer", &layer);
  tAlignmentEKLMModule->SetBranchAddress("sector", &sector);
  tAlignmentEKLMModule->SetBranchAddress("param", &param);
  tAlignmentEKLMModule->SetBranchAddress("value", &value);
  tAlignmentEKLMModule->SetBranchAddress("error", &error);
  tComparisonEKLMModule->Branch("section", &section, "section/I");
  tComparisonEKLMModule->Branch("layer", &layer, "layer/I");
  tComparisonEKLMModule->Branch("sector", &sector, "sector/I");
  tComparisonEKLMModule->Branch("param", &param, "param/I");
  tComparisonEKLMModule->Branch("value0", &value0, "value0/F");
  tComparisonEKLMModule->Branch("value", &value, "value/F");
  tComparisonEKLMModule->Branch("error", &error, "error/F");
  if (displacementFile != nullptr) {
    n = tDisplacementEKLMModule->GetEntries();
    for (i = 0; i < n; i++) {
      tDisplacementEKLMModule->GetEntry(i);
      val0[section - 1][layer - 1][sector - 1][0][0][param - 1] =
        value;
    }
  }
  n = tAlignmentEKLMModule->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignmentEKLMModule->GetEntry(i);
    if (displacementFile != nullptr) {
      value0 =
        val0[section - 1][layer - 1][sector - 1][0][0][param - 1];
    } else {
      value0 = 0;
    }
    tComparisonEKLMModule->Fill();
  }
  /* Segment. */
  if (displacementFile != nullptr) {
    tDisplacementEKLMSegment->SetBranchAddress("section", &section);
    tDisplacementEKLMSegment->SetBranchAddress("layer", &layer);
    tDisplacementEKLMSegment->SetBranchAddress("sector", &sector);
    tDisplacementEKLMSegment->SetBranchAddress("plane", &plane);
    tDisplacementEKLMSegment->SetBranchAddress("segment", &segment);
    tDisplacementEKLMSegment->SetBranchAddress("param", &param);
    tDisplacementEKLMSegment->SetBranchAddress("value", &value);
  }
  tAlignmentEKLMSegment->SetBranchAddress("section", &section);
  tAlignmentEKLMSegment->SetBranchAddress("layer", &layer);
  tAlignmentEKLMSegment->SetBranchAddress("sector", &sector);
  tAlignmentEKLMSegment->SetBranchAddress("plane", &plane);
  tAlignmentEKLMSegment->SetBranchAddress("segment", &segment);
  tAlignmentEKLMSegment->SetBranchAddress("param", &param);
  tAlignmentEKLMSegment->SetBranchAddress("value", &value);
  tAlignmentEKLMSegment->SetBranchAddress("error", &error);
  tComparisonEKLMSegment->Branch("section", &section, "section/I");
  tComparisonEKLMSegment->Branch("layer", &layer, "layer/I");
  tComparisonEKLMSegment->Branch("sector", &sector, "sector/I");
  tComparisonEKLMSegment->Branch("plane", &plane, "plane/I");
  tComparisonEKLMSegment->Branch("segment", &segment, "segment/I");
  tComparisonEKLMSegment->Branch("param", &param, "param/I");
  tComparisonEKLMSegment->Branch("value0", &value0, "value0/F");
  tComparisonEKLMSegment->Branch("value", &value, "value/F");
  tComparisonEKLMSegment->Branch("error", &error, "error/F");
  if (displacementFile != nullptr) {
    n = tDisplacementEKLMSegment->GetEntries();
    for (i = 0; i < n; i++) {
      tDisplacementEKLMSegment->GetEntry(i);
      val0[section - 1][layer - 1][sector - 1][plane - 1][segment - 1]
        [param - 1] = value;
    }
  }
  n = tAlignmentEKLMSegment->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignmentEKLMSegment->GetEntry(i);
    if (displacementFile != nullptr) {
      value0 = val0[section - 1][layer - 1][sector - 1][plane - 1][segment - 1]
                   [param - 1];
    } else {
      value0 = 0;
    }
    tComparisonEKLMSegment->Fill();
  }
  /* BKLM module. */
  tAlignmentBKLMModule->SetBranchAddress("section", &section);
  tAlignmentBKLMModule->SetBranchAddress("layer", &layer);
  tAlignmentBKLMModule->SetBranchAddress("sector", &sector);
  tAlignmentBKLMModule->SetBranchAddress("param", &param);
  tAlignmentBKLMModule->SetBranchAddress("value", &value);
  tAlignmentBKLMModule->SetBranchAddress("error", &error);
  tComparisonBKLMModule->Branch("section", &section, "section/I");
  tComparisonBKLMModule->Branch("layer", &layer, "layer/I");
  tComparisonBKLMModule->Branch("sector", &sector, "sector/I");
  tComparisonBKLMModule->Branch("param", &param, "param/I");
  tComparisonBKLMModule->Branch("value0", &value0, "value0/F");
  tComparisonBKLMModule->Branch("value", &value, "value/F");
  tComparisonBKLMModule->Branch("error", &error, "error/F");
  n = tAlignmentBKLMModule->GetEntries();
  for (i = 0; i < n; i++) {
    tAlignmentBKLMModule->GetEntry(i);
    value0 = 0;
    tComparisonBKLMModule->Fill();
  }
  /* Write data. */
  fComparison->cd();
  tComparisonEKLMModule->Write();
  tComparisonEKLMSegment->Write();
  tComparisonBKLMModule->Write();
  delete tComparisonEKLMSegment;
  delete tComparisonEKLMModule;
  delete tComparisonBKLMModule;
  delete fComparison;
  delete tAlignmentEKLMSegment;
  delete tAlignmentEKLMModule;
  delete tAlignmentBKLMModule;
  delete fAlignment;
  if (displacementFile != nullptr) {
    delete tDisplacementEKLMSegment;
    delete tDisplacementEKLMModule;
    delete fDisplacement;
  }
}
