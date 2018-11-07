/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "drawHitmap.h"

using namespace std;

void drawHitmap()
{
  // Add the debug file from the BKLMUnpacker module
  TChain* tree = new TChain("hits");
  //tree->Add("inputFile.root")
  tree->Add("bklm_ut3_20181009.root");
  //tree->Add("bklm_phase2_r5184.root");

  // Create name for the pdf file
  TString date = "20181009";
  TString comment = "";
  TString pdfName = "hits_ut3_" + date + comment + ".pdf";

  // List of variables
  int run;
  int axis;
  int charge;
  int sector;
  int tdc;
  float tdcnano;
  int layer;
  int isForward;
  int channel;
  int rawchannel;
  int ctimediff;
  int format;
  int lane;
  long int time;

  tree->SetBranchAddress("run", &run);
  tree->SetBranchAddress("format", &format);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("isForward", &isForward);
  tree->SetBranchAddress("sector", &sector);
  tree->SetBranchAddress("layer", &layer);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("rawChannel", &rawchannel);
  tree->SetBranchAddress("axis", &axis);
  tree->SetBranchAddress("ctime_diff", &ctimediff);
  tree->SetBranchAddress("charge", &charge);
  tree->SetBranchAddress("time", &time);
  tree->SetBranchAddress("tdc", &tdc);
  tree->SetBranchAddress("tdc_nano", &tdcnano);

  // Create name for histograms, including run number
  char rname[50];
  for (int entry = 0; entry < 1; entry++) {
    tree->GetEntry(entry);
    sprintf(rname, " Run%i", run);
  }
  TString runNum(rname);
  TString title = date + " "  + runNum;

  // Define global histograms for BF and BB
  TH2F* histSectLayer_BF = createTH2("SectLayBF", title + " -- BF", 31, -0.5, 15., "Layer (0-based)", 17, -0.5, 8., "BF", 0);
  TH2F* histSectLayer_BB = createTH2("SectLayBB", title + " -- BB", 31, -0.5, 15., "Layer (0-based)", 17, -0.5, 8., "BB", 0);

  // Define histograms for each sector
  TH2F* histZ[2][8];
  TH2F* histPhi[2][8];
  TH1F* histTdc_RPC[2][8];
  TH1F* histTdc_Sci[2][8];
  TH1F* histCTime_RPC[2][8];
  TH1F* histCTime_Sci[2][8];
  TH2F* histCharge[2][8][2][2];
  TH1F* histLayer[2][8];
  TH1F* histAxis[2][8];

  for (int fb = 0; fb < 2; fb++) {
    for (int iSector = 0; iSector < 8; iSector++) {
      char hname[50];

      if (fb == 0)
        sprintf(hname, " BB%i z", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i z", iSector);
      histZ[fb][iSector] = createTH2(hname, title + " -- " + hname, 34, -1.5, 15.5, "Layer", 132, -1.5, 64.5, "Channel", 0);

      if (fb == 0)
        sprintf(hname, " BB%i phi", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i phi", iSector);
      histPhi[fb][iSector] = createTH2(hname, title + " -- " + hname, 34, -1.5, 15.5, "Layer", 132, -1.5, 64.5, "Channel", 0);

      if (fb == 0)
        sprintf(hname, " BB%i RPC TDC", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i RPC TDC", iSector);
      histTdc_RPC[fb][iSector] = createTH1(hname, title + " -- " + hname, 100, -10, 2200, "TDC", "", 1);

      if (fb == 0)
        sprintf(hname, " BB%i Sci TDC", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i Sci TDC", iSector);
      histTdc_Sci[fb][iSector] = createTH1(hname, title + " -- " + hname, 100, 0, 26, "TDC", "", 1);

      if (fb == 0)
        sprintf(hname, " BB%i RPC time diff.", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i RPC time diff.", iSector);
      histCTime_RPC[fb][iSector] = createTH1(hname, title + " -- " + hname, 500, -2000, 2000, "RPC_cTime - trg_time", "", 1);

      if (fb == 0)
        sprintf(hname, " BB%i Sci time diff.", iSector);
      if (fb == 1)
        sprintf(hname, " BF%i Sci time diff.", iSector);
      histCTime_Sci[fb][iSector] = createTH1(hname, title + " -- " + hname, 500, -200, -100, "Sci_cTime - trg_time", "", 1);

      for (int iLayer = 0; iLayer < 2; iLayer++) {
        for (int iAxis = 0; iAxis < 2; iAxis++) {
          if (fb == 0 && iAxis == 0)
            sprintf(hname, " BB%i Layer%i z ", iSector, iLayer);
          if (fb == 1 && iAxis == 0)
            sprintf(hname, " BF%i Layer%i z ", iSector, iLayer);
          if (fb == 0 && iAxis == 1)
            sprintf(hname, " BB%i Layer%i phi ", iSector, iLayer);
          if (fb == 1 && iAxis == 1)
            sprintf(hname, " BF%i Layer%i phi ", iSector, iLayer);
          histCharge[fb][iSector][iLayer][iAxis] = createTH2(hname, title + " -- " + hname, 130, 0, 65, "Channel", 100, -500, 10, "Charge",
                                                             0);
        } // iAxis
      } // iLayer

      //if (fb==0)
      //sprintf(hname, " BB%i ", iSector);
      //if (fb==1)
      //sprintf(hname, " BF%i ", iSector);
      //histLayer[fb][iSector] = createTH1(hname, title + " -- " + hname, 32, -1.5, 15.5, "Layer (0-based)", "", 1);

      //if (fb==0)
      //sprintf(hname, " BB%i axis", iSector);
      //if (fb==1)
      //sprintf(hname, " BF%i axis", iSector);
      //histAxis[fb][iSector] = createTH1(hname, title + " -- " + hname, 10, 0, 2, "Axis", "", 1);

    } // iSector
  } // fb

  // Fill the histograms
  for (int entry = 0; entry < tree->GetEntries(); entry++) {
    tree->GetEntry(entry);
    if ((entry % 1000) == 0)
      cout << "Processed " << entry << " entries " << endl;

    if (format == 1)
      continue;
    if (lane == 0)
      continue;

    if (isForward == 1)
      histSectLayer_BF->Fill(layer - 1, sector - 1);
    if (isForward == 0)
      histSectLayer_BB->Fill(layer - 1, sector - 1);
    if (axis == 0)
      histZ[isForward][sector - 1]->Fill(layer - 1, rawchannel - 1);
    if (axis == 1)
      histPhi[isForward][sector - 1]->Fill(layer - 1, rawchannel - 1);
    if (layer > 2) {
      histCTime_RPC[isForward][sector - 1]->Fill(ctimediff);
      histTdc_RPC[isForward][sector - 1]->Fill(tdc);
    }
    if (layer < 3) {
      histCTime_Sci[isForward][sector - 1]->Fill(ctimediff);
      histTdc_Sci[isForward][sector - 1]->Fill(tdc);
    }
    if (charge < -20) {
      histCharge[isForward][sector - 1][layer - 1][axis]->Fill(rawchannel - 1, charge);
    }
    //histLayer[isForward][sector-1]->Fill(layer-1);
    //histAxis[isForward][sector-1]->Fill(axis);
  } // entry

  // Draw the histograms and save them in the pdf file
  TCanvas* c0_BF = new TCanvas("c0_BF", "", 1000, 800);
  c0_BF->cd();
  histSectLayer_BF->Draw("COLZ");
  c0_BF->SaveAs(pdfName + "(");

  TCanvas* c0_BB = new TCanvas("c0_BB", "", 1000, 800);
  c0_BB->cd();
  histSectLayer_BB->Draw("COLZ");
  c0_BB->SaveAs(pdfName);

  TCanvas* c1 = new TCanvas("c1", "", 1000, 800);
  c1->Divide(2);

  TCanvas* c2 = new TCanvas("c2", "", 1000, 800);
  c2->Divide(2, 2);

  TCanvas* c3 = new TCanvas("c3", "", 1000, 800);
  c3->Divide(2, 2);

  for (int fb = 1; fb > -1; fb--) {
    for (int iSector = 0; iSector < 8; iSector++) {
      c1->cd(1);
      histZ[fb][iSector]->Draw("COLZ");
      c1->cd(2);
      histPhi[fb][iSector]->Draw("COLZ");
      c1->SaveAs(pdfName);

      c2->cd(1);
      histTdc_RPC[fb][iSector]->Draw("HIST");
      histTdc_RPC[fb][iSector]->SetMinimum(0);
      c2->cd(2);
      histTdc_Sci[fb][iSector]->Draw("HIST");
      histTdc_Sci[fb][iSector]->SetMinimum(0);
      c2->cd(3);
      histCTime_RPC[fb][iSector]->Draw("HIST");
      histCTime_RPC[fb][iSector]->SetMinimum(0);
      c2->cd(4);
      histCTime_Sci[fb][iSector]->Draw("HIST");
      histCTime_Sci[fb][iSector]->SetMinimum(0);
      c2->SaveAs(pdfName);

      c3->cd(1);
      histCharge[fb][iSector][0][0]->Draw("COLZ");
      c3->cd(2);
      histCharge[fb][iSector][0][1]->Draw("COLZ");
      c3->cd(3);
      histCharge[fb][iSector][1][0]->Draw("COLZ");
      c3->cd(4);
      histCharge[fb][iSector][1][1]->Draw("COLZ");
      c3->SaveAs(pdfName);

      //histLayer[fb][iSector]->Draw();
      //histLayer[fb][iSector]->SetMinimum(0);
      //histAxis[fb][iSector]->Draw();
      //histAxis[fb][iSector]->SetMinimum(0);
    } // fb
  } // iSector

  // Last page of the pdf file, empty
  TCanvas* c4 = new TCanvas("c4", "", 1000, 800);
  c4->cd();
  c4->SaveAs(pdfName + ")");

  gSystem->Exit(0);
}
