/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisV0.cc
// Description : Overlay plotting for V0
//
// Author : Bryan Fulsom (PNNL), B Spruck
// Date : 2019-01-17
//-


#include <dqm/analysis/modules/DQMHistAnalysisV0.h>

#include <TFile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TH2.h>
#include <TGraph.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisV0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisV0Module::DQMHistAnalysisV0Module()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("OverlayPath", m_OverlayPath, "Path to CAD drawings", std::string(""));

}


void DQMHistAnalysisV0Module::initialize()
{
  B2INFO("DQMHistAnalysisV0: initialized.");

  gROOT->cd();
  for (int i = 0; i < 32; i++) {
    m_c_xvsy[i] = new TCanvas(Form("V0Object/c_xvsy[%i]", i), Form("c_xvsy[%i]", i), 800, 800);
  }

  m_c_xvsz = new TCanvas("V0Object/c_xvsz", "c_xvsz", 1500, 400); //Stretch in x to enhance visibility

  auto* m_fh = new TFile(Form("%s/v0cad.root", m_OverlayPath.c_str()));

  contLevelXY.resize(32);
  for (int i = 0; i < 32; i++) {
    contLevelXY[i] = new TList();
    m_fh->cd();
    if (m_fh->cd(Form("h_%dc", i))) {
      for (int j = 0; j < 500; j++) {
        auto curv = gDirectory->Get(Form("Graph_%d", j));
        if (!curv) break;
        contLevelXY[i]->AddLast(curv);
      }
    }
  }
  {
    contLevelXZ = new TList();
    m_fh->cd();
    if (m_fh->cd("h_xzc")) {
      for (int j = 0; true; j++) {
        auto curv = gDirectory->Get(Form("Graph_%d", j));
        if (!curv) break;
        contLevelXZ->AddLast(curv);
      }
    }
  }
  /// m_fh.Close(); ? need to clone curves??
  gROOT->cd();
}


void DQMHistAnalysisV0Module::event()
{

  //gStyle requirements
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kViridis, 0, 0.7);
  for (int i = 0; i < 32; i++) {
    TH2* h = (TH2*) findHist(Form("V0Objects/xvsy[%i]", i));
    m_c_xvsy[i]->cd();
    if (h) h->Draw("COLZ");

    TList* c = contLevelXY[i];
    if (c && c->GetSize() > 0) {
      auto* curv = (TGraph*)c->First();
      for (int j = 0; j < c->GetSize(); j++) {
        //auto* gc = (TGraph*)curv->Clone();
        //gc->Draw("C");
        curv->Draw("L");
        curv = (TGraph*)c->After(curv); // Get Next graph
      }
    }

    m_c_xvsy[i]->Modified();
    m_c_xvsy[i]->Update();
  }

  TH2* hxz = (TH2*) findHist("V0Objects/xvsz");
  m_c_xvsz->cd();

  // create a new pad for every event? -> mem leak!

  m_c_xvsz->cd();
  if (hxz) hxz->Draw("COLZ");
  {
    TList* c = contLevelXZ;
    if (c && c->GetSize() > 0) {
      auto* curv = (TGraph*)c->First();
      for (int j = 0; j < c->GetSize(); j++) {
        //auto* gc = (TGraph*)curv->Clone();
        //gc->Draw("C");
        curv->Draw("L");
        curv = (TGraph*)c->After(curv); // Get Next graph
      }
    }
  }

  m_c_xvsz->Modified();
  m_c_xvsz->Update();

}

