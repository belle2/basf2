
void ViewAlignment(const char *fname)
{
	int endcap, layer, sector, plane, segment, param;
	float value0, value, error;
	int i, n;
	TH1F *hDx = new TH1F("h_dx", "", 14, 0.5, 14.5);
	TH1F *hDx0 = new TH1F("h_dx0", "", 14, 0.5, 14.5);
	hDx->GetXaxis()->SetTitle("Layer");
	hDx->GetYaxis()->SetTitle("#Deltax, cm");
	hDx0->SetMarkerStyle(20);
	hDx0->SetMarkerColor(2);
	TCanvas *c = new TCanvas();
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
		if (endcap == 1 && sector == 1 && param == 1) {
			hDx->SetBinContent(layer, value);
			hDx->SetBinError(layer, error);
			hDx0->SetBinContent(layer, value0);
		}
	}
	hDx->SetMinimum(-0.5);
	hDx->SetMaximum(4.5);
	hDx->Draw("e");
	hDx0->Draw("same p");
	c->Print("alignment.eps");
}

