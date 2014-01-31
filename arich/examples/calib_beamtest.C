{
  thit = (TTree*)_file0->Get("hits");
  h2 = new TH2F("h2","h2",150,-3.14,3.14,150,0.2,0.4);
  thit->Draw("thc:fic>>h2","agel==0");
  fa1 = new TF1("fa1","gaus + pol1(3)",0.25,0.34);
  fa1->SetParameters(110,0.3,0.015,45,0);
  h2->FitSlicesY(fa1);
  h2_1 = (TH1D*)gDirectory->Get("h2_1");
  c1 = new TCanvas("c1");
  c1->Divide(2,2);
  c1->cd(1);
  h2_1->Draw();
  fa2 = new TF1("fa2","[0]+[1]*sin(x-[2])",-3.14,3.14);
  h2_1->Fit(fa2);
  h2_1->SetMaximum(0.4);   h2_1->SetMinimum(0.2);
  c1->cd(2);
  h2_2 = (TH1D*)gDirectory->Get("h2_2");
  h2_2->Draw();
  h2_2->SetMaximum(0.02);   h2_2->SetMinimum(0.0);
  c1->cd(3);
  h2->Draw("COLZ");
   
  return 0;
}
