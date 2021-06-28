/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/modules/onlineEventT0/OnlineEventT0CreatorModule.h>


using namespace Belle2;

REG_MODULE(OnlineEventT0Creator)

OnlineEventT0CreatorModule::OnlineEventT0CreatorModule() : Module()
{
  setDescription("Module to write the EventT0s computed on HLT");

  addParam("OnlineEventT0Name", m_onlineEventT0Name, "Name of the OnlineEventT0 StoreArray", std::string(m_onlineEventT0Name));
  addParam("eventT0Name", m_eventT0Name, "Name of the EventT0 StoreArray", std::string(m_eventT0Name));

  setPropertyFlags(c_ParallelProcessingCertified);
}

void OnlineEventT0CreatorModule::initialize()
{

  m_onlineEventT0.registerInDataStore(m_onlineEventT0Name);

  m_eventT0.isRequired(m_eventT0Name);

}

void OnlineEventT0CreatorModule::event()
{
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot write OnlineEventT0");
    return;
  }

  if (m_onlineEventT0.isValid()) {
    B2DEBUG(20, "OnlineEventT0 object already present, do nothing");
    return;
  }

  // check if a TOP hypothesis exists
  auto topHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::TOP);

  if (topHypos.size() == 0) {
    B2DEBUG(20, "No TOP time hypothesis available");
  } else {
    // get the most accurate TOP eventT0 (there is only one)
    const auto topBestT0 = topHypos.back();
    m_onlineEventT0.appendNew(topBestT0.eventT0, topBestT0.eventT0Uncertainty, Const::EDetector::TOP);
  }

  // check if a CDC hypothesis exists
  auto cdcHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);

  if (cdcHypos.size() == 0) {
    B2DEBUG(20, "No CDC time hypothesis available");
  } else {
    // get the most accurate CDC evenT0 (latest)
    const auto cdcBestT0 = cdcHypos.back();
    m_onlineEventT0.appendNew(cdcBestT0.eventT0, cdcBestT0.eventT0Uncertainty, Const::EDetector::CDC);
  }


  // check if ECL hypothesis exists
  auto eclHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL);

  if (eclHypos.size() == 0) {
    B2DEBUG(20, "No ECL t0 hypothesis available, exiting");
  } else {
    // get the most accurate ECL evenT0 (smallest chi2/quality)
    double bestECL_quality = eclHypos[0].quality ;
    int bestECL_idx = 0 ;
    for (int i = 0; i < (int)eclHypos.size(); i++) {
      if (eclHypos[i].quality < bestECL_quality) {
        bestECL_quality = eclHypos[i].quality ;
        bestECL_idx = i ;
      }
    }
    m_onlineEventT0.appendNew(eclHypos[bestECL_idx].eventT0, eclHypos[bestECL_idx].eventT0Uncertainty, Const::EDetector::ECL);

  }

}


