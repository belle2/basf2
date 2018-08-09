/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTOP.h>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTOP)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTOPModule::DQMHistAnalysisTOPModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(20, "DQMHistAnalysisTOP: Constructor done.");
}


DQMHistAnalysisTOPModule::~DQMHistAnalysisTOPModule() { }

void DQMHistAnalysisTOPModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");

  m_c_goodHitsMean = new TCanvas("c_good_hits_mean");
  m_c_goodHitsRMS = new TCanvas("c_good_hits_rms");
  m_c_badHitsMean = new TCanvas("c_bad_hits_mean");
  m_c_badHitsRMS = new TCanvas("c_bad_hits_rms");

  m_h_goodHitsMean = new TH1F("h_good_hits_mean", "Mean of good hits per event", 16, 0.5, 16.5);
  m_h_goodHitsRMS = new TH1F("h_good_hits_rms", "RMS of good hits per event", 16, 0.5, 16.5);
  m_h_badHitsMean = new TH1F("h_bad_hits_mean", "Mean of bad hits per event", 16, 0.5, 16.5);
  m_h_badHitsRMS = new TH1F("h_bad_hits_rms", "RMS of bad hits per event", 16, 0.5, 16.5);

  m_h_goodHitsMean->GetXaxis()->SetTitle("slot no.");
  m_h_goodHitsMean->GetYaxis()->SetTitle("hits / event");
  m_h_goodHitsRMS->GetXaxis()->SetTitle("slot no.");
  m_h_goodHitsRMS->GetYaxis()->SetTitle("hits / event");

  m_h_badHitsMean->GetXaxis()->SetTitle("slot no.");
  m_h_badHitsMean->GetYaxis()->SetTitle("hits / event");
  m_h_badHitsRMS->GetXaxis()->SetTitle("slot no.");
  m_h_badHitsRMS->GetYaxis()->SetTitle("hits / event");
}


void DQMHistAnalysisTOPModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}

void DQMHistAnalysisTOPModule::event()
{
  for (int i = 1; i <= 16; i++) {
    string hname1 = str(format("good_hits_per_event%1%") % (i));;
    string hname2 = str(format("bad_hits_per_event%1%") % (i));;
    TH1* h1 = findHist(hname1);
    TH1* h2 = findHist(hname2);
    if (h1 != NULL && h2 != NULL) {
      m_h_goodHitsMean->SetBinContent(i, h1->GetMean());
      m_h_goodHitsRMS->SetBinContent(i, h1->GetRMS());
      m_h_badHitsMean->SetBinContent(i, h2->GetMean());
      m_h_badHitsRMS->SetBinContent(i, h2->GetRMS());
    } else {
      m_h_goodHitsMean->SetBinContent(i, 0);
      m_h_goodHitsRMS->SetBinContent(i, 0);
      m_h_badHitsMean->SetBinContent(i, 0);
      m_h_badHitsRMS->SetBinContent(i, 0);
      B2INFO("DQMHistAnalysisTOP: cant find histograms.");
    }
  }
  m_c_goodHitsMean->Clear();
  m_c_goodHitsMean->cd();
  m_h_goodHitsMean->Draw();
  m_c_goodHitsMean->Modified();

  m_c_goodHitsRMS->Clear();
  m_c_goodHitsRMS->cd();
  m_h_goodHitsRMS->Draw();
  m_c_goodHitsRMS->Modified();

  m_c_badHitsMean->Clear();
  m_c_badHitsMean->cd();
  m_h_badHitsMean->Draw();
  m_c_badHitsMean->Modified();

  m_c_badHitsRMS->Clear();
  m_c_badHitsRMS->cd();
  m_h_badHitsRMS->Draw();
  m_c_badHitsRMS->Modified();
}

void DQMHistAnalysisTOPModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");
}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

