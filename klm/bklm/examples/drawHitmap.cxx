/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

void drawHitmap(TString inputFile)
{ 
  TFile *file = new TFile(inputFile.Data());
  if (file->IsZombie()) {
    cout << "The input file is not working!" << endl;
    return;
  }

  TTree* metaData = (TTree *)file->Get("extraInfo");
  int exp = -1;
  metaData->SetBranchAddress("exp", &exp);
  int run = -1;
  metaData->SetBranchAddress("run", &run);
  int year = -1;
  metaData->SetBranchAddress("year", &year);
  int month = -1;
  metaData->SetBranchAddress("month", &month);
  int day = -1;
  metaData->SetBranchAddress("day", &day);
  int hour = -1;
  metaData->SetBranchAddress("hour", &hour);
  int min = -1;
  metaData->SetBranchAddress("min", &min);
  int sec = -1;
  metaData->SetBranchAddress("sec", &sec);
  metaData->GetEntry(0);

  TString runNumber;
  runNumber.Form("%d", run);
  
  TCanvas* firstCanvas = new TCanvas("firstCanvas", "firstCanvas", 800, 800);
  TPaveText *text = new TPaveText(.05,.4,.95,.9);
  TString string;
  text->AddText("BKLM Hitmap");
  string.Form("Exp. %d; Run %d", exp, run);
  text->AddText(string.Data());
  text->AddText("");
  text->AddText("File generated");
  string.Form("on %d/%d/%d", year, month, day);
  text->AddText(string.Data());
  string.Form("at %d:%d:%d UTC", hour, min, sec);
  text->AddText(string.Data());
  text->Draw();
  firstCanvas->SaveAs("run" + runNumber + ".pdf(");
  
  TH2F* hist = 0;
  
  TCanvas* canvasGeneral = new TCanvas("canvasGeneral", "canvasGeneral", 800, 800);
  TCanvas* canvasDetailed = new TCanvas("canvasDetailed", "canvasDetailed", 800, 800);
  canvasDetailed->Divide(2,1);
  gStyle->SetOptStat(0);

  canvasGeneral->cd();
  file->GetObject("SectLayBF", hist);
  hist->Draw("colz");
  canvasGeneral->SaveAs("run" + runNumber + ".pdf");

  for(int i = 0; i < 8; i++)
  {
    TString histoName;
    /*
    canvasDetailed->cd(1);
    histoName.Form("PlaneZBF%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");
    
    canvasDetailed->cd(2);
    histoName.Form("PlanePhiBF%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");

    canvasDetailed->SaveAs("run" + runNumber + ".pdf");
    */
    canvasDetailed->cd(1);
    histoName.Form("PlaneZStripBF%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");
    
    canvasDetailed->cd(2);
    histoName.Form("PlanePhiStripBF%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");

    canvasDetailed->SaveAs("run" + runNumber + ".pdf");    
  }

  canvasGeneral->cd();
  file->GetObject("SectLayBB", hist);
  hist->Draw("colz");
  canvasGeneral->SaveAs("run" + runNumber + ".pdf");

  for(int i = 0; i < 8; i++)
  {
    TString histoName;
    /*
    canvasDetailed->cd(1);
    histoName.Form("PlaneZBB%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");
 
    canvasDetailed->cd(2);
    histoName.Form("PlanePhiBB%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");

    canvasDetailed->SaveAs("run" + runNumber + ".pdf");
    */
    canvasDetailed->cd(1);
    histoName.Form("PlaneZStripBB%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");
    
    canvasDetailed->cd(2);
    histoName.Form("PlanePhiStripBB%i", i);
    file->GetObject(histoName.Data(), hist);
    hist->Draw("colz");

    canvasDetailed->SaveAs("run" + runNumber + ".pdf");    
  }

  TCanvas* empty = new TCanvas("empty", "empty", 800, 800);
  empty->SaveAs("run" + runNumber + ".pdf)");

  file->Close();
  gSystem->Exit(0);
}
