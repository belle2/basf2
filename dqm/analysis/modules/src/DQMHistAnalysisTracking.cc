//+
// File : DQMHistAnalysisTracking.cc
// Description : Analysis of Tracking
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisTracking.h>
#include <TROOT.h>
#include <TLatex.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTrackingModule::DQMHistAnalysisTrackingModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("Tracking"));
  B2DEBUG(99, "DQMHistAnalysisTracking: Constructor done.");
}

void DQMHistAnalysisTrackingModule::initialize()
{
  gROOT->cd(); // this seems to be important, or strange things happen

  m_cTrackingError = new TCanvas((m_histogramDirectoryName + "/c_TrackingError").data());

  B2DEBUG(99, "DQMHistAnalysisTracking: initialized.");
}

void DQMHistAnalysisTrackingModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisTracking: beginRun called.");

  m_cTrackingError->Clear();
  m_cTrackingError->SetLogz();
}

void DQMHistAnalysisTrackingModule::event()
{
  bool error_flag = false;
  bool warn_flag = false;
  if (!m_cTrackingError) return;

  std::string name = "NumberTrackingErrorFlags";

  TH1* hh1 = findHist(name);
  if (hh1 == NULL) {
    hh1 = findHist(m_histogramDirectoryName, name);
  }
  if (hh1) {
    hh1->SetStats(false);

    double nGood = hh1->GetBinContent(1);
    double nBad = hh1->GetBinContent(2);
    double ratio = (nGood > 1) ? (nBad / nGood) : 1;
    error_flag |= (ratio > 5e-2); /// TODO level might need adjustment
    warn_flag |= (ratio > 1e-2); /// TODO level might need adjustment

    m_cTrackingError->cd();

    if (error_flag) {
      m_cTrackingError->Pad()->SetFillColor(kRed);// Red
    } else if (warn_flag) {
      m_cTrackingError->Pad()->SetFillColor(kYellow);// Yellow
    } else if (nBad < 10) {
      m_cTrackingError->Pad()->SetFillColor(kGreen);// Green
    } else {
      m_cTrackingError->Pad()->SetFillColor(kWhite);// White
    }

    hh1->Draw("hist");
  }
//   auto tt = new TLatex(5.5, 3, "1.3.2 Module is broken, please ignore");
//   tt->SetTextAngle(90);// Rotated
//   tt->SetTextAlign(12);// Centered
//   tt->Draw();

  m_cTrackingError->Modified();
  m_cTrackingError->Update();
}


