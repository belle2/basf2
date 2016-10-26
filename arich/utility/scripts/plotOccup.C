/*
 * Plots histograms from ARICHHists module 
 * By: Luka Santelj
 */

void plotOccup(){
  
  int map[6] = {5,6,4,3,2,1};

  gStyle->SetPalette(1);
  
  TH1F* hBits = (TH1F*)_file0->Get("hBits"); 
  TH1F* hHits = (TH1F*)_file0->Get("hHits"); 
  TH2Poly* hits2d = (TH2Poly*)_file0->Get("hits");
  TString fname(_file0->GetName());
  std::cout << fname << std::endl;
  TCanvas* cc = new TCanvas("cc","cc", 1000,1000);
  cc->Divide(3,3);
  cc->cd(4);
  hHits->GetXaxis()->SetRange(hHits->FindFirstBinAbove(0)-3,hHits->FindLastBinAbove(0)+3);
  hHits->Draw();
  cc->cd(7);

  hBits->SetMinimum(0);
  hBits->Draw();
  int nevt = int(hHits->GetEntries());
  TPaveText *pt = new TPaveText(.05,.1,.95,.8);
  pt->SetTextAlign(10);
  pt->AddText("");
  pt->AddText(TString::Format("# of events: %i",nevt));
  pt->AddText("");

  pt->AddText("hits  (hits/event)");
  pt->AddText("");
  double max = 0;
  TH2F* h2D[6];
  for(int i=0; i<6; i++){
    //cc->cd(i+5);
    TString hname("HAPD "); hname+=map[i];
    //TH2F* h2D = (TH2F*)_file0->Get(hname);
    h2D[i] = (TH2F*)_file0->Get(hname);
    h2D[i]->SetStats(false);
    //h2D->Draw("colz");
    if(h2D[i]->GetBinContent(h2D[i]->GetMaximumBin()) > max) max = h2D[i]->GetBinContent(h2D[i]->GetMaximumBin());
    pt->AddText(TString::Format("%s:   %i   (%.1f)", hname.Data(), int(h2D[i]->GetEntries()),h2D[i]->GetEntries()/nevt));
  }
  
  int map1[6] = {2,3,5,6,8,9};
  for(int i=0; i<6; i++){
    //cc->cd(i+5);
    cc->cd(map1[i]);
    h2D[i]->SetMaximum(max);
    h2D[i]->SetMinimum(0);
    h2D[i]->Draw("colz");
  }
   
  cc->cd(1);
  pt->Draw();

  TCanvas* c2 = new TCanvas("c2","c2", 500,500);  
  hits2d->GetXaxis()->SetRangeUser(25,60);
  hits2d->GetYaxis()->SetRangeUser(25,60);
  hits2d->SetStats(false);

  hits2d->Draw();
  
}
