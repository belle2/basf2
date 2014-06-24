void plot_voltage(const char* filename) {
  gStyle->SetOptStat(0);
  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetOptStat("");
  gStyle->SetStripDecimals(kFALSE);
  gStyle->SetPadTickX(kTRUE);
  gStyle->SetPadTickY(kTRUE);
  gStyle->SetPalette(1);
  gStyle->SetNumberContours(99);

  const TDatime da1(2014,06,19,14,30,00);
  const TDatime da2(2014,06,19,15,30,00);
  TChain* tree = new TChain("hv_status");
  tree->Add(filename);
  tree->Draw("voltage_mon:record_time", 
	     "crate == 0 && slot == 1 && channel == 1");
  TGraph *gr = new TGraph(tree->GetSelectedRows(),
			  tree->GetV2(), tree->GetV1());
  gr->SetLineColor(2);
  gr->SetLineWidth(2);
  gr->SetMarkerStyle(20);
  gr->SetMarkerColor(2);
  gr->SetMarkerSize(0.5);
  TH1* h = new TH1D("hframe", ";Time; Voltage", 10, 
		    da1.Convert(), da2.Convert());
  h->SetMinimum(0);
  h->SetMaximum(2500);
  h->GetXaxis()->SetTimeDisplay(1);
  h->GetYaxis()->SetTitleOffset(1.25);
  h->Draw("");
  gr->Draw("PLsame");

  TLegend* leg = new TLegend(0.62,0.92,0.98,0.98);
  leg->SetFillColor(kWhite);
  leg->SetLineWidth(1);
  leg->SetShadowColor(0);
  leg->AddEntry(gr, "Monitored (Slot, Channel) = (1,1)", "l");
  leg->Draw();
}
