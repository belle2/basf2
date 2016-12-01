/*
 * Plots histograms from ARICHDQM module 
 * run as: "root -l histogram_file.root plotDQM.C" 
 * By: Luka Santelj
 */

void plotDQM(){
  
  gStyle->SetPalette(1);
  
  TH1F* hBits = (TH1F*)_file0->Get("ARICH/hBits"); 
  TH1F* hHits = (TH1F*)_file0->Get("ARICH/hHits"); 
  TH2F* hHitsHapd = (TH2F*)_file0->Get("ARICH/hHitsChn"); 
  TH1F* hHitsMerger = (TH1F*)_file0->Get("ARICH/hHitsMerg"); 
  TH1F* hHitsCopper = (TH1F*)_file0->Get("ARICH/hHitsCopp"); 
  
  // create 2D hit map using ARICHChannelHist
  Belle2::ARICHChannelHist* chHits = new Belle2::ARICHChannelHist("chHits","# of hits/channel/(1000 events)");
  for(int i=1;i<421;i++){
    for(int j=0;j<144;j++){
      chHits->setBinContent(i,j,hHitsHapd->GetBinContent(i,j+1));
    }
  }  
  
  TCanvas* cc = new TCanvas("cc","cc", 1000,500);
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
  cc->cd(6);
  hHitsCopper->Draw();
  
}
