/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTracking.cc
// Description : Analysis of Tracking
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisTracking.h>
#include <TROOT.h>

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
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("TracksDQM"));
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
  if (!m_cTrackingError) return;

  std::string name = "NumberTrackingErrorFlags";

  TH1* hh1 = findHist(name);
  if (hh1 == NULL) {
    hh1 = findHist(m_histogramDirectoryName, name);
  }
  if (hh1) {
    hh1->SetStats(true); // maybe we want the mean?

    bool error_flag = false;
    bool warn_flag = false;
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
  m_cTrackingError->Modified();
  m_cTrackingError->Update();
}


