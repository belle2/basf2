/*
 * Plots histograms from ARICHDQM module 
 * run as: "root -l histogram_file.root plotDQM.C" 
 * By: Luka Santelj
 */

void plotDQM(){
  
  gStyle->SetPalette(1);
  
  TH1F* hBits = (TH1F*)_file0->Get("ARICHDQM/h_bits"); 
  TH1F* hHits = (TH1F*)_file0->Get("ARICHDQM/h_hitsPerEvent"); 
  TH1F* hHitsHapd = (TH1F*)_file0->Get("ARICHDQM/h_chHit"); 
  TH1F* hHitsMerger = (TH1F*)_file0->Get("ARICHDQM/h_mergerHit");
  TH1F* htheta = (TH1F*)_file0->Get("ARICHDQM/h_theta");
  TH1F* hHitsPerTrack = (TH1F*)_file0->Get("ARICHDQM/h_hitsPerTrack"); 
  TH2F* hHitsHAPDperEvent = (TH2F*)_file0->Get("ARICHDQM/h_hapdHitPerEvent"); 
  
  // create 2D hit map using ARICHChannelHist
  Belle2::ARICHChannelHist* chHits = new Belle2::ARICHChannelHist("chHits","# of hits/channel");
  for(int i=1;i<421;i++){
    for(int j=0;j<144;j++){
      int ch = (i-1)*144 + j;
      chHits->setBinContent(i,j,hHitsHapd->GetBinContent(ch+1));
    }
  }  
  
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
