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

  setDescription("Make data quality monitoring plots for dE/dx: means and resolutions for bhabha samples, band plots for lepton/hadron samples.");
}

CDCDedxDQMModule::~CDCDedxDQMModule() { }

void CDCDedxDQMModule::defineHisto()
{
  B2INFO("Creating a ROOT file for CDC dE/dx DQM...");

  m_h_dedxmeans = new TH1F("h_dedxmeans", "CDC dE/dx electron truncated mean", 100, 0.0, 2.0);
  m_h_dedxbands = new TH2F("h_dedxbands", "CDC dE/dx band plots", 100, 0.0, 3.0, 100, 0.0, 10.0);
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

    m_h_dedxmeans->Fill(dedxTrack.getDedx());
    m_h_dedxbands->Fill(dedxTrack.getMomentum(), dedxTrack.getDedx());
  }
}

void CDCDedxDQMModule::endRun()
{
}

void CDCDedxDQMModule::terminate()
{
}
