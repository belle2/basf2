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
    m_img[i] = TImage::Open(Form("%s/%ic.png", m_OverlayPath.c_str(), i));
    m_img[i]->SetConstRatio(kTRUE);
  }


  m_c_xvsz = new TCanvas("c_xvsz", "c_xvsz", 1500, 400); //Stretch in x to enhance visibility
  m_img_xz = TImage::Open(Form("%s/xzc.png", m_OverlayPath.c_str()));
  m_img_xz->SetConstRatio(kTRUE);


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
    m_c_xvsy[i]->cd();
    m_img[i]->Draw("x");

    p = new TPad("p", "p", 0, 0, 1, 1);
    p->SetLogz();
    p->SetFillStyle(4000);
    p->SetFrameFillStyle(4000);

    m_c_xvsy[i]->cd();
    p->Draw();
    p->cd();
    if (h) h->Draw("COLZ");

    m_c_xvsy[i]->Modified();
    m_c_xvsy[i]->Update();
  }

  TH2* hxz = (TH2*) findHist("V0Objects/xvsz");
  m_c_xvsz->cd();
  m_img_xz->Draw("x");

  pxz = new TPad("pxz", "pxz", 0, 0, 1, 1);
  pxz->SetLogz();
  pxz->SetFillStyle(4000);
  pxz->SetFrameFillStyle(4000);

  m_c_xvsz->cd();
  pxz->Draw();
  pxz->cd();
  if (hxz) hxz->Draw("COLZ");

  m_c_xvsz->Modified();
  m_c_xvsz->Update();

}

void DQMHistAnalysisV0Module::endRun()
{
}


void DQMHistAnalysisV0Module::terminate()
{
  B2INFO("DQMHistAnalysisV0: terminated.");
}

