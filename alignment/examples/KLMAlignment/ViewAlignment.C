
void ViewAlignment(const char *fname)
{
  const int nEndcaps = 2;
  const int nSectors = 4;
  int endcap, layer, sector, plane, segment, param;
  float value0, value, error;
  int i, j, n;
  char str[128];
  TH1F *hDx[nEndcaps][nSectors], *hDx0[nEndcaps][nSectors];
  TH1F *hDy[nEndcaps][nSectors], *hDy0[nEndcaps][nSectors];
  TH1F *hDalpha[nEndcaps][nSectors], *hDalpha0[nEndcaps][nSectors];
  gStyle->SetTitleXSize(0.05);
  gStyle->SetTitleYSize(0.05);
  gStyle->SetLabelSize(0.05);
  gStyle->SetLabelSize(0.05, "y");
  gStyle->SetTitleFont(42);
  gStyle->SetTitleFont(42, "y");
  gStyle->SetLabelFont(42);
  gStyle->SetLabelFont(42,"y");
  gStyle->SetTitleOffset(1.0);
  gStyle->SetTitleOffset(1.5,"y");
  gStyle->SetFillColor(10);
  TCanvas *c = new TCanvas();
  c->SetLeftMargin(0.15);
  for (i = 0; i < nEndcaps; i++) {
    for (j = 0; j < nSectors; j++) {
      snprintf(str, 128, "h_dx_%d_%d", i + 1, j + 1);
      hDx[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      snprintf(str, 128, "h_dx0_%d_%d", i + 1, j + 1);
      hDx0[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      hDx[i][j]->GetXaxis()->SetTitle("Layer");
      hDx[i][j]->GetYaxis()->SetTitle("#Deltax, cm");
      hDx0[i][j]->SetMarkerStyle(20);
      hDx0[i][j]->SetMarkerColor(2);
      snprintf(str, 128, "h_dy_%d_%d", i + 1, j + 1);
      hDy[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      snprintf(str, 128, "h_dy0_%d_%d", i + 1, j + 1);
      hDy0[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      hDy[i][j]->GetXaxis()->SetTitle("Layer");
      hDy[i][j]->GetYaxis()->SetTitle("#Deltay, cm");
      hDy0[i][j]->SetMarkerStyle(20);
      hDy0[i][j]->SetMarkerColor(2);
      snprintf(str, 128, "h_dalpha_%d_%d", i + 1, j + 1);
      hDalpha[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      snprintf(str, 128, "h_dalpha0_%d_%d", i + 1, j + 1);
      hDalpha0[i][j] = new TH1F(str, "", 14, 0.5, 14.5);
      hDalpha[i][j]->GetXaxis()->SetTitle("Layer");
      hDalpha[i][j]->GetYaxis()->SetTitle("#Delta#alpha, rad");
      hDalpha0[i][j]->SetMarkerStyle(20);
      hDalpha0[i][j]->SetMarkerColor(2);
    }
  }
  TFile *f = new TFile(fname);
  TTree *tComparisonSector = (TTree*)f->Get("eklm_sector");
  tComparisonSector->SetBranchAddress("endcap", &endcap);
  tComparisonSector->SetBranchAddress("layer", &layer);
  tComparisonSector->SetBranchAddress("sector", &sector);
  tComparisonSector->SetBranchAddress("param", &param);
  tComparisonSector->SetBranchAddress("value", &value);
  tComparisonSector->SetBranchAddress("value0", &value0);
  tComparisonSector->SetBranchAddress("error", &error);
  n = tComparisonSector->GetEntries();
  for (i = 0; i < n; i++) {
    tComparisonSector->GetEntry(i);
    switch (param) {
      case 1:
        hDx[endcap - 1][sector - 1]->SetBinContent(layer, value);
        hDx[endcap - 1][sector - 1]->SetBinError(layer, error);
        hDx0[endcap - 1][sector - 1]->SetBinContent(layer, value0);
        break;
      case 2:
        hDy[endcap - 1][sector - 1]->SetBinContent(layer, value);
        hDy[endcap - 1][sector - 1]->SetBinError(layer, error);
        hDy0[endcap - 1][sector - 1]->SetBinContent(layer, value0);
        break;
      case 6:
        hDalpha[endcap - 1][sector - 1]->SetBinContent(layer, value);
        hDalpha[endcap - 1][sector - 1]->SetBinError(layer, error);
        hDalpha0[endcap - 1][sector - 1]->SetBinContent(layer, value0);
        break;
    }
  }
  for (i = 0; i < nEndcaps; i++) {
    for (j = 0; j < nSectors; j++) {
      hDx[i][j]->SetMinimum(-4.5);
      hDx[i][j]->SetMaximum(4.5);
      hDx[i][j]->Draw("e");
      hDx0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_dx_%d_%d.eps", i + 1, j + 1);
      c->Print(str);
      hDy[i][j]->SetMinimum(-4.5);
      hDy[i][j]->SetMaximum(4.5);
      hDy[i][j]->Draw("e");
      hDy0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_dy_%d_%d.eps", i + 1, j + 1);
      c->Print(str);
      hDalpha[i][j]->SetMinimum(-0.02);
      hDalpha[i][j]->SetMaximum(0.02);
      hDalpha[i][j]->Draw("e");
      hDalpha0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_dalpha_%d_%d.eps", i + 1, j + 1);
      c->Print(str);
    }
  }
}

