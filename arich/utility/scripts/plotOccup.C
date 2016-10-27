/*
 * Plots histograms from ARICHDQM module 
 * run as: "root -l histogram_file.root plotOccup.C" 
 * By: Luka Santelj
 */

void plotOccup(){
  
  gStyle->SetPalette(1);
  
  TH1F* hBits = (TH1F*)_file0->Get("hBits"); 
  TH1F* hHits = (TH1F*)_file0->Get("hHits"); 
  TH2F* hHitsHapd = (TH2F*)_file0->Get("hHitsChn"); 
  TH1F* hHitsMerger = (TH1F*)_file0->Get("hHitsMerg"); 
  TH1F* hHitsCopper = (TH1F*)_file0->Get("hHitsCopp"); 
  TH2Poly* hits2d = (TH2Poly*)_file0->Get("hits");
  TString fname(_file0->GetName());
  std::cout << fname << std::endl;
  TCanvas* cc = new TCanvas("cc","cc", 1000,500);
  cc->Divide(3,2);
  cc->cd(1);
  hHits->GetXaxis()->SetRange(hHits->FindFirstBinAbove(0)-3,hHits->FindLastBinAbove(0)+3);
  hHits->Draw();
  cc->cd(2);

  hBits->SetMinimum(0);
  hBits->Draw();
  int nevt = int(hHits->GetEntries());

  cc->cd(3);
  hits2d->SetStats(false);
  hits2d->Draw();
  
  cc->cd(4);
  hHitsHapd->Draw();
  cc->cd(5);
  hHitsMerger->Draw();
  cc->cd(6);
  hHitsCopper->Draw();
  
}
