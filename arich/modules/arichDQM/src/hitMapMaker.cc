/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <arich/modules/arichDQM/hitMapMaker.h>

// ARICH
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dataobjects/ARICHHit.h>

#include <mdst/dataobjects/Track.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TColor.h>
#include <TExec.h>

#include <vector>
#include <fstream>

using namespace std;

namespace Belle2 {

  TH2* moduleHitMap(TH1* hitMap, int moduleID)
  {

    int m_moduleID = moduleID;

    DBObjPtr<ARICHChannelMapping> arichChMap;

    TH2* m_moduleHitMap = new TH2D(Form("HAPDHitMapMod%d", moduleID), Form("HAPD hit map module %d;nChX;nChY", moduleID), 12, 0.5, 12.5,
                                   12,
                                   0.5, 12.5);
    TH1* m_hitMap = hitMap;

    for (int i = 0; i < 144; i++) {
      int hitsNum = m_hitMap->GetBinContent((m_moduleID - 1) * 144 + i);
      int xChn, yChn;
      arichChMap->getXYFromAsic(i, xChn, yChn);
      if (hitsNum != 0) {
        m_moduleHitMap->Fill(xChn + 1, yChn + 1, hitsNum);
      }
    }

    return m_moduleHitMap;
  }

  TH2* moduleDeadMap(TH1* hitMap, int moduleID)
  {
    int m_moduleID = moduleID;

    DBObjPtr<ARICHChannelMapping> arichChMap;

    TH2* m_moduleDeadMap = new TH2D(Form("HAPDDeadMapMod%d", moduleID), Form("HAPD alive/dead module %d;nChX;nChY", moduleID), 2, 0.5,
                                    2.5,
                                    2, 0.5, 2.5);
    TH1* m_hitMap = hitMap;

    int deadCh[2][2] = {};

    for (int i = 0; i < 144; i++) {
      int hitsNum = m_hitMap->GetBinContent((m_moduleID - 1) * 144 + i);
      int xChn, yChn;
      arichChMap->getXYFromAsic(i, xChn, yChn);
      if (hitsNum == 0) deadCh[(int)xChn / 6][(int)yChn / 6]++;
    }

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        if (deadCh[j][k] > 18) {
          m_moduleDeadMap->Fill(j + 1, k + 1, 1);
        } else {
          m_moduleDeadMap->Fill(j + 1, k + 1, 10);
        }
      }
    }

    return m_moduleDeadMap;
  }

  TH1* mergerClusterHitMap1D(TH1* hitMap, int mergerID)
  {

    int m_mergerID = mergerID;

    DBObjPtr<ARICHChannelMapping> arichChMap;
    DBObjPtr<ARICHMergerMapping> arichMergerMap;

    TH1* m_hitMap = hitMap;

    std::vector<int> moduleIDs;
    for (int i = 1; i < 7; i++) {
      moduleIDs.push_back(arichMergerMap->getModuleID(m_mergerID, i));
    }

    TH1D* m_mergerHitMap1D = new TH1D("MergerHitMap1D", Form("Hit map in Merger Board %d", m_mergerID), 144 * 6, -0.5, 144 * 6 - 0.5);
    for (int i = 1; i < 7; i++) {
      for (int j = 0; j < 144; j++) {
        int hitsNum = m_hitMap->GetBinContent((moduleIDs[i] - 1) * 144 + i);
        m_mergerHitMap1D->Fill(144 * (i - 1) + j, hitsNum);
      }
    }
    return m_mergerHitMap1D;
  }

  TCanvas* mergerClusterHitMap2D(TH1* hitMap, int mergerID)
  {
    int m_mergerID = mergerID;

    DBObjPtr<ARICHChannelMapping> arichChMap;
    DBObjPtr<ARICHMergerMapping> arichMergerMap;

    TH1* m_hitMap = hitMap;


    std::vector<int> moduleIDs;
    for (int i = 1; i < 7; i++) {
      moduleIDs.push_back(arichMergerMap->getModuleID(m_mergerID, i));
    }

    TCanvas* m_mergerHitMap = new TCanvas("MergerHitMap", "Hit map in Merger Board", 600, 400);
    m_mergerHitMap->Divide(3, 2);
    for (int i = 1; i < 7; i++) {
      m_mergerHitMap->cd(i);
      moduleHitMap(m_hitMap, moduleIDs[i])->Draw("coloz");
    }
    return m_mergerHitMap;
  }

  TCanvas* sectorHitMap(TH1* hitMap, int sector)
  {
    //    TH1* m_hitMap = NULL;
    TH2* m_moduleHitMap = NULL;

    //m_hitMap = hitMap;
    TPad* p_hitMaps[421] = {};
    TCanvas* m_sectorHitMap = new TCanvas(Form("c_hitMap%d", sector - 1), Form("Hit map of sector%d", sector - 1), 600, 400);
    for (int i = 1; i < 421; i++) {
      for (int iRing = 0; iRing < 7; iRing++) {
        if (((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) < i && i <= ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector)) {
          m_sectorHitMap->cd();
          p_hitMaps[i] = new TPad(Form("p_hitMap%d", i), "",
                                  (double)((double)(6 - iRing) / 2 + ((i - ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) - 1) % (iRing + 7))) / 13,
                                  (double)iRing / 7, (double)((double)(8 - iRing) / 2 + ((i - ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) - 1) %
                                                              (iRing + 7))) / 13, (double)(iRing + 1) / 7);
          p_hitMaps[i]->Draw();
          p_hitMaps[i]->SetNumber(i);
          m_sectorHitMap->cd(i);
          m_moduleHitMap = moduleHitMap(hitMap, i);
          m_moduleHitMap->SetTitleSize(0, "xyz");
          m_moduleHitMap->SetTitle(0);
          m_moduleHitMap->SetLineWidth(1);
          gStyle->SetLabelColor(0, "xyz");
          m_moduleHitMap->SetStats(0);
          m_moduleHitMap->Draw("col");
        }
      }
    }
    return m_sectorHitMap;
  }

  TCanvas* sectorDeadMap(TH1* hitMap, int sector)
  {
    //TH1* m_hitMap = NULL;
    TH2* m_moduleDeadMap = NULL;
    TExec* ex1 = NULL;

    //m_hitMap = hitMap;
    TPad* p_hitMaps[421] = {};
    TCanvas* m_sectorDeadMap = new TCanvas(Form("c_deadMap%d", sector - 1), Form("Dead chip map of sector%d", sector - 1), 600, 400);
    for (int i = 1; i < 421; i++) {
      for (int iRing = 0; iRing < 7; iRing++) {
        if (((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) < i && i <= ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector)) {
          m_sectorDeadMap->cd();
          p_hitMaps[i] = new TPad(Form("p_deadMap%d", i), "",
                                  (double)((double)(6 - iRing) / 2 + ((i - ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) - 1) % (iRing + 7))) / 13,
                                  (double)iRing / 7, (double)((double)(8 - iRing) / 2 + ((i - ((iRing + 13)*iRing / 2) * 6 + (iRing + 7) * (sector - 1) - 1) %
                                                              (iRing + 7))) / 13, (double)(iRing + 1) / 7);
          p_hitMaps[i]->Draw();
          p_hitMaps[i]->SetNumber(i);
          m_sectorDeadMap->cd(i);
          m_moduleDeadMap = moduleDeadMap(hitMap, i);
          m_moduleDeadMap->SetTitleSize(0, "xyz");
          m_moduleDeadMap->SetTitle(0);
          m_moduleDeadMap->SetLineWidth(1);
          m_moduleDeadMap->SetStats(0);
          gStyle->SetLabelColor(0, "xyz");
          ex1 = new TExec("ex1", "deadPalette();");
          ex1->Draw();
          m_moduleDeadMap->Draw("colz");
        }
      }
    }
    return m_sectorDeadMap;
  }

  void deadPalette()
  {
    static Int_t colors[50];
    static Bool_t initialized = kFALSE;
    Double_t Red[3]    = { 1.00, 0.00, 0.00};
    Double_t Green[3]  = { 0.00, 0.00, 0.00};
    Double_t Blue[3]   = { 0.00, 1.00, 1.00};
    Double_t Length[3] = { 0.00, 0.20, 1.00 };
    if (!initialized) {
      Int_t FI = TColor::CreateGradientColorTable(3, Length, Red, Green, Blue, 50);
      for (int i = 0; i < 50; i++) colors[i] = FI + i;
      initialized = kTRUE;
      return;
    }
    gStyle->SetPalette(50, colors);
  }

}
