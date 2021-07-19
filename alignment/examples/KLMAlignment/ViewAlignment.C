/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

void ViewAlignment(const char *fname)
{
  const int nEndcaps = 2;
  const int nSectors = 4;
  const int nBKLMSectors = 8;
  int section, layer, sector, param;
  float value0, value, error;
  int i, j, n;
  char str[128];
  TH1F *hDx[nEndcaps][nSectors], *hDx0[nEndcaps][nSectors];
  TH1F *hDy[nEndcaps][nSectors], *hDy0[nEndcaps][nSectors];
  TH1F *hDalpha[nEndcaps][nSectors], *hDalpha0[nEndcaps][nSectors];
  TH1F *hBKLMDx[nEndcaps][nBKLMSectors], *hBKLMDx0[nEndcaps][nBKLMSectors];
  TH1F *hBKLMDy[nEndcaps][nBKLMSectors], *hBKLMDy0[nEndcaps][nBKLMSectors];
  TH1F *hBKLMDalpha[nEndcaps][nBKLMSectors],
       *hBKLMDalpha0[nEndcaps][nBKLMSectors];
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
    for (j = 0; j < nBKLMSectors; j++) {
      snprintf(str, 128, "h_bklm_dx_%d_%d", i + 1, j + 1);
      hBKLMDx[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      snprintf(str, 128, "h_bklm_dx0_%d_%d", i + 1, j + 1);
      hBKLMDx0[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      hBKLMDx[i][j]->GetXaxis()->SetTitle("Layer");
      hBKLMDx[i][j]->GetYaxis()->SetTitle("#Deltax, cm");
      hBKLMDx0[i][j]->SetMarkerStyle(20);
      hBKLMDx0[i][j]->SetMarkerColor(2);
      snprintf(str, 128, "h_bklm_dy_%d_%d", i + 1, j + 1);
      hBKLMDy[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      snprintf(str, 128, "h_bklm_dy0_%d_%d", i + 1, j + 1);
      hBKLMDy0[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      hBKLMDy[i][j]->GetXaxis()->SetTitle("Layer");
      hBKLMDy[i][j]->GetYaxis()->SetTitle("#Deltay, cm");
      hBKLMDy0[i][j]->SetMarkerStyle(20);
      hBKLMDy0[i][j]->SetMarkerColor(2);
      snprintf(str, 128, "h_bklm_dalpha_%d_%d", i + 1, j + 1);
      hBKLMDalpha[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      snprintf(str, 128, "h_bklm_dalpha0_%d_%d", i + 1, j + 1);
      hBKLMDalpha0[i][j] = new TH1F(str, "", 15, 0.5, 15.5);
      hBKLMDalpha[i][j]->GetXaxis()->SetTitle("Layer");
      hBKLMDalpha[i][j]->GetYaxis()->SetTitle("#Delta#alpha, rad");
      hBKLMDalpha0[i][j]->SetMarkerStyle(20);
      hBKLMDalpha0[i][j]->SetMarkerColor(2);
    }
  }
  /* EKLM. */
  TFile *f = new TFile(fname);
  TTree *tComparisonEKLMModule = (TTree*)f->Get("eklm_module");
  tComparisonEKLMModule->SetBranchAddress("section", &section);
  tComparisonEKLMModule->SetBranchAddress("layer", &layer);
  tComparisonEKLMModule->SetBranchAddress("sector", &sector);
  tComparisonEKLMModule->SetBranchAddress("param", &param);
  tComparisonEKLMModule->SetBranchAddress("value", &value);
  tComparisonEKLMModule->SetBranchAddress("value0", &value0);
  tComparisonEKLMModule->SetBranchAddress("error", &error);
  n = tComparisonEKLMModule->GetEntries();
  for (i = 0; i < n; i++) {
    tComparisonEKLMModule->GetEntry(i);
    switch (param) {
      case 1:
        hDx[section - 1][sector - 1]->SetBinContent(layer, value);
        hDx[section - 1][sector - 1]->SetBinError(layer, error);
        hDx0[section - 1][sector - 1]->SetBinContent(layer, value0);
        break;
      case 2:
        hDy[section - 1][sector - 1]->SetBinContent(layer, value);
        hDy[section - 1][sector - 1]->SetBinError(layer, error);
        hDy0[section - 1][sector - 1]->SetBinContent(layer, value0);
        break;
      case 6:
        hDalpha[section - 1][sector - 1]->SetBinContent(layer, value);
        hDalpha[section - 1][sector - 1]->SetBinError(layer, error);
        hDalpha0[section - 1][sector - 1]->SetBinContent(layer, value0);
        break;
    }
  }
  for (i = 0; i < nEndcaps; i++) {
    for (j = 0; j < nSectors; j++) {
      hDx[i][j]->SetMinimum(-4.5);
      hDx[i][j]->SetMaximum(4.5);
      hDx[i][j]->Draw("e");
      hDx0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_eklm_%d_%d_dx.eps", i + 1, j + 1);
      c->Print(str);
      hDy[i][j]->SetMinimum(-4.5);
      hDy[i][j]->SetMaximum(4.5);
      hDy[i][j]->Draw("e");
      hDy0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_eklm_%d_%d_dy.eps", i + 1, j + 1);
      c->Print(str);
      hDalpha[i][j]->SetMinimum(-0.02);
      hDalpha[i][j]->SetMaximum(0.02);
      hDalpha[i][j]->Draw("e");
      hDalpha0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_eklm_%d_%d_dalpha.eps", i + 1, j + 1);
      c->Print(str);
    }
  }
  /* BKLM. */
  TTree *tComparisonBKLMModule = (TTree*)f->Get("bklm_module");
  tComparisonBKLMModule->SetBranchAddress("section", &section);
  tComparisonBKLMModule->SetBranchAddress("layer", &layer);
  tComparisonBKLMModule->SetBranchAddress("sector", &sector);
  tComparisonBKLMModule->SetBranchAddress("param", &param);
  tComparisonBKLMModule->SetBranchAddress("value", &value);
  tComparisonBKLMModule->SetBranchAddress("value0", &value0);
  tComparisonBKLMModule->SetBranchAddress("error", &error);
  n = tComparisonBKLMModule->GetEntries();
  for (i = 0; i < n; i++) {
    tComparisonBKLMModule->GetEntry(i);
    switch (param) {
      case 1:
        hBKLMDx[section][sector - 1]->SetBinContent(layer, value);
        hBKLMDx[section][sector - 1]->SetBinError(layer, error);
        hBKLMDx0[section][sector - 1]->SetBinContent(layer, value0);
        break;
      case 2:
        hBKLMDy[section][sector - 1]->SetBinContent(layer, value);
        hBKLMDy[section][sector - 1]->SetBinError(layer, error);
        hBKLMDy0[section][sector - 1]->SetBinContent(layer, value0);
        break;
      case 6:
        hBKLMDalpha[section][sector - 1]->SetBinContent(layer, value);
        hBKLMDalpha[section][sector - 1]->SetBinError(layer, error);
        hBKLMDalpha0[section][sector - 1]->SetBinContent(layer, value0);
        break;
    }
  }
  for (i = 0; i < nEndcaps; i++) {
    for (j = 0; j < nBKLMSectors; j++) {
      hBKLMDx[i][j]->SetMinimum(-4.5);
      hBKLMDx[i][j]->SetMaximum(4.5);
      hBKLMDx[i][j]->Draw("e");
      hBKLMDx0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_bklm_%d_%d_dx.eps", i + 1, j + 1);
      c->Print(str);
      hBKLMDy[i][j]->SetMinimum(-4.5);
      hBKLMDy[i][j]->SetMaximum(4.5);
      hBKLMDy[i][j]->Draw("e");
      hBKLMDy0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_bklm_%d_%d_dy.eps", i + 1, j + 1);
      c->Print(str);
      hBKLMDalpha[i][j]->SetMinimum(-0.02);
      hBKLMDalpha[i][j]->SetMaximum(0.02);
      hBKLMDalpha[i][j]->Draw("e");
      hBKLMDalpha0[i][j]->Draw("same p");
      snprintf(str, 128, "alignment_bklm_%d_%d_dalpha.eps", i + 1, j + 1);
      c->Print(str);
    }
  }
}

