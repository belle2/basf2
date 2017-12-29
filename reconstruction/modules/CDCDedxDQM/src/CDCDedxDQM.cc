/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>

using namespace Belle2;

REG_MODULE(CDCDedxDQM)

CDCDedxDQMModule::CDCDedxDQMModule() : HistoModule()
{

  setDescription("Make data quality monitoring plots for dE/dx.");
}

CDCDedxDQMModule::~CDCDedxDQMModule() { }

void CDCDedxDQMModule::defineHisto()
{
  B2INFO("Creating a ROOT file for CDC dE/dx DQM...");

  m_h_dedx = new TH1F("h_dedxmean", "CDC dE/dx truncated mean", 100, 0.0, 2.0);
  m_h_dedxmean = new TH1F("h_dedxmean", "CDC dE/dx average truncated mean", 100, 0.0, 2.0);
  m_h_dedxsigma = new TH1F("h_dedxmean", "CDC dE/dx resolution", 100, 0.0, 2.0);
}

void CDCDedxDQMModule::initialize()
{
  // register in datastore
  m_cdcDedxTracks.isRequired();

  REG_HISTOGRAM   // required to register histograms to HistoManager
}

void CDCDedxDQMModule::beginRun()
{
}

void CDCDedxDQMModule::event()
{
  // **************************************************
  //
  //  LOOP OVER EACH DEDX MEASUREMENT (TRACK LEVEL)
  //
  // **************************************************

  for (auto& dedxTrack : m_cdcDedxTracks) {
    if (dedxTrack.size() == 0) {
      B2WARNING("No good hits on this track...");
      continue;
    }

    m_h_dedx->Fill(dedxTrack.getTruncatedMean());
  }
}

void CDCDedxDQMModule::endRun()
{
  m_h_dedx->Fit("gaus");
  double mean = m_h_dedx->GetFunction("gaus")->GetParameter(1);
  double sigma = m_h_dedx->GetFunction("gaus")->GetParameter(2);
  m_h_dedxmean->Fill(mean);
  m_h_dedxsigma->Fill(sigma);
}

void CDCDedxDQMModule::terminate()
{
}
