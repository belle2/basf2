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

  m_c_particleHitsMean = new TCanvas("c_particle_hits_mean");
  m_c_particleHitsRMS = new TCanvas("c_particle_hits_rms");
  m_c_otherHitsMean = new TCanvas("c_other_hits_mean");
  m_c_otherHitsRMS = new TCanvas("c_other_hits_rms");

  m_h_particleHitsMean = new TH1F("h_particle_hits_mean", "h_particle_hits_mean", 16, 0.5, 16.5);
  m_h_particleHitsRMS = new TH1F("h_particle_hits_rms", "h_particle_hits_rms", 16, 0.5, 16.5);
  m_h_otherHitsMean = new TH1F("h_other_hits_mean", "h_other_hits_mean", 16, 0.5, 16.5);
  m_h_otherHitsRMS = new TH1F("h_other_hits_rms", "h_other_hits_rms", 16, 0.5, 16.5);
}


void DQMHistAnalysisTOPModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}

void DQMHistAnalysisTOPModule::event()
{
  for (int i = 1; i <= 16; i++) {
    string hname1 = str(format("particle_hits_per_event%1%") % (i));;
    string hname2 = str(format("other_hits_per_event%1%") % (i));;
    TH1* h1 = findHist(hname1);
    TH1* h2 = findHist(hname2);
    if (h1 != NULL && h2 != NULL) {
      m_h_particleHitsMean->SetBinContent(i, h1->GetMean());
      m_h_particleHitsRMS->SetBinContent(i, h1->GetRMS());
      m_h_otherHitsMean->SetBinContent(i, h2->GetMean());
      m_h_otherHitsRMS->SetBinContent(i, h2->GetRMS());
    } else {
      m_h_particleHitsMean->SetBinContent(i, 0);
      m_h_particleHitsRMS->SetBinContent(i, 0);
      m_h_otherHitsMean->SetBinContent(i, 0);
      m_h_otherHitsRMS->SetBinContent(i, 0);
      B2INFO("DQMHistAnalysisTOP: cant find histograms.");
    }
  }
  m_c_particleHitsMean->Clear();
  m_c_particleHitsMean->cd();
  m_h_particleHitsMean->Draw();
  m_c_particleHitsMean->Modified();
  m_c_particleHitsRMS->Clear();
  m_c_particleHitsRMS->cd();
  m_h_particleHitsRMS->Draw();
  m_c_particleHitsRMS->Modified();

  m_c_otherHitsMean->Clear();
  m_c_otherHitsMean->cd();
  m_h_otherHitsMean->Draw();
  m_c_otherHitsMean->Modified();
  m_c_otherHitsRMS->Clear();
  m_c_otherHitsRMS->cd();
  m_h_otherHitsRMS->Draw();
  m_c_otherHitsRMS->Modified();
}

void DQMHistAnalysisTOPModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");
}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

