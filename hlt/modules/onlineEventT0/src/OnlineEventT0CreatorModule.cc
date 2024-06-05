/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <hlt/modules/onlineEventT0/OnlineEventT0CreatorModule.h>

/* Basf2 headers. */
#include <framework/core/Environment.h>
#include <framework/logging/LogConfig.h>

using namespace Belle2;

REG_MODULE(OnlineEventT0Creator);

OnlineEventT0CreatorModule::OnlineEventT0CreatorModule() : Module()
{
  setDescription("Module to write the EventT0s computed on the online systems (HLT). "
                 "By default, this module is run only on the online systems unless the user "
                 "specifically requires to execute it.");
  addParam("executeThisModule", m_executeThisModule,
           "Execute this module: if True, this module is executed durig the reconstruction, otherwise not.", false);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void OnlineEventT0CreatorModule::initialize()
{
  // If we are running online, execute the module regardless of what the user specified.
  LogConfig::ELogRealm realm = Environment::Instance().getRealm();
  if (realm == LogConfig::c_Online)
    m_executeThisModule = true;
  // Run the usual checks only if the module is actually executed.
  if (m_executeThisModule) {
    m_onlineEventT0.registerInDataStore();
    m_eventT0.isRequired();
  }
}

void OnlineEventT0CreatorModule::event()
{
  if (!m_executeThisModule) {
    B2DEBUG(20, "OnlineEventT0 is not executed as requested");
    return;
  }
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot write OnlineEventT0");
    return;
  }
  if (m_onlineEventT0.getEntries() > 0) {
    B2DEBUG(20, "OnlineEventT0 object already present, do nothing");
    return;
  }
  // check if ECL hypothesis exists
  const auto bestECLHypo = m_eventT0->getBestECLTemporaryEventT0();
  if (not bestECLHypo) {
    B2DEBUG(20, "No ECL EventT0 available");
  } else {
    m_onlineEventT0.appendNew(bestECLHypo->eventT0, bestECLHypo->eventT0Uncertainty, Const::EDetector::ECL);
  }
  // check if a CDC hypothesis exists
  const auto bestCDCHypo = m_eventT0->getBestCDCTemporaryEventT0();
  if (not bestCDCHypo) {
    B2DEBUG(20, "No CDC EventT0 available");
  } else {
    m_onlineEventT0.appendNew(bestCDCHypo->eventT0, bestCDCHypo->eventT0Uncertainty, Const::EDetector::CDC);
  }
  // check if a TOP hypothesis exists
  const auto bestTOPHypo = m_eventT0->getBestTOPTemporaryEventT0();
  if (not bestTOPHypo) {
    B2DEBUG(20, "No TOP EventT0 available");
  } else {
    m_onlineEventT0.appendNew(bestTOPHypo->eventT0, bestTOPHypo->eventT0Uncertainty, Const::EDetector::TOP);
  }
  // check if a SVD hypothesis exists
  const auto bestSVDHypo = m_eventT0->getBestSVDTemporaryEventT0();
  if (not bestSVDHypo) {
    B2DEBUG(20, "No SVD EventT0 available");
  } else {
    m_onlineEventT0.appendNew(bestSVDHypo->eventT0, bestSVDHypo->eventT0Uncertainty, Const::EDetector::SVD);
  }
}
