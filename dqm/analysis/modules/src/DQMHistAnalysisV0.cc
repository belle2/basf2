//+
// File : DQMHistAnalysisV0.cc
// Description :
//
// Author : Bryan Fulsom (PNNL)
// Date : 2019-01-17
//-


#include <dqm/analysis/modules/DQMHistAnalysisV0.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TPaletteAxis.h>
#include <TAxis.h>
#include <TH2.h>
#include <TPad.h>
#include <TImage.h>

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


DQMHistAnalysisV0Module::~DQMHistAnalysisV0Module() { }

void DQMHistAnalysisV0Module::initialize()
{
  B2INFO("DQMHistAnalysisV0: initialized.");

  gROOT->cd();
  for (int i = 0; i < 32; i++) {
    m_c_xvsy[i] = new TCanvas(Form("c_xvsy[%i]", i), Form("c_xvsy[%i]", i), 800, 800);
  }
}


void DQMHistAnalysisV0Module::beginRun()
{
}


void DQMHistAnalysisV0Module::event()
{

  //gStyle requirements
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kViridis, 0, 0.7);

  for (int i = 0; i < 32; i++) {
    TH2* h = (TH2*) findHist(Form("V0Objects/xvsy[%i]", i));
    TImage* img = TImage::Open(Form("%s/%ic.png", m_OverlayPath.c_str(), i));
    img->SetConstRatio(kTRUE);
    m_c_xvsy[i]->cd();
    img->Draw("x");

    TPad* p = new TPad("p", "p", 0, 0, 1, 1);
    p->SetLogz();
    p->SetFillStyle(4000);
    p->SetFrameFillStyle(4000);
    p->Draw();
    p->cd();
    if (h) h->Draw("COLZ");

    m_c_xvsy[i]->Modified();
    m_c_xvsy[i]->Update();
  }


}

void DQMHistAnalysisV0Module::endRun()
{
}


void DQMHistAnalysisV0Module::terminate()
{
  B2INFO("DQMHistAnalysisV0: terminated.");
}

