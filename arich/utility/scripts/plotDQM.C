/*
 * Plots histograms from ARICHDQM module output file 
 * run as: "root -l histogram_file.root plotDQM.C" 
 * By: Luka Santelj
 */

void plotDQM(){
  
  gStyle->SetPalette(1);
  TGaxis::SetMaxDigits(4); 

  TH1F* hBits = (TH1F*)_file0->Get("ARICH/bits"); 
  TH1F* hHits = (TH1F*)_file0->Get("ARICH/hitsPerEvent"); 
  TH1F* hHitsHapd = (TH1F*)_file0->Get("ARICH/chHit"); 
  TH1F* hHitsMerger = (TH1F*)_file0->Get("ARICH/mergerHit");
  TH1F* htheta = (TH1F*)_file0->Get("ARICH/theta");
  TH1F* hHitsPerTrack = (TH1F*)_file0->Get("ARICH/hitsPerTrack"); 
  TH2F* hHitsHAPDperEvent = (TH2F*)_file0->Get("ARICH/hapdHitPerEvent"); 
  
  // create 2D hit map using ARICHChannelHist
  Belle2::ARICHChannelHist* chHits = new Belle2::ARICHChannelHist("chHits1","# of hits/channel");
  chHits->fillFromTH1(hHitsHapd);

  TCanvas* cc = new TCanvas("cc","cc", 1200,600);
  cc->Divide(3,2);
  cc->cd(1);
  hHits->GetXaxis()->SetRange(hHits->FindFirstBinAbove(0)-3,hHits->FindLastBinAbove(0)+3);
  hHits->Draw();
  cc->cd(2);

  hBits->SetMinimum(0);
  hBits->Draw();

  cc->cd(3);
  chHits->SetStats(false);
  chHits->Draw();
  cc->Update();
  TPaletteAxis *palette = (TPaletteAxis*)chHits->GetListOfFunctions()->FindObject("palette");
  palette->SetX2NDC(0.93);
  
  cc->cd(4);
  hHitsHapd->Draw();
  cc->cd(5);
  hHitsMerger->Draw();

  TCanvas* c2 = new TCanvas("c2","c2",500,500);
  chHits->Draw();
  cc->cd(6);
  hHitsHAPDperEvent->Draw("colz");

  TCanvas* c3 = new TCanvas("c3","c3",800,400);
  c3->Divide(2);
  c3->cd(1);
  hHitsPerTrack->Draw();
  c3->cd(2);
  htheta->Draw();
  
}
