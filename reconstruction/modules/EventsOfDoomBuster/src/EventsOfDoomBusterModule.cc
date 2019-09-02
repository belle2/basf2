/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/EventsOfDoomBuster/EventsOfDoomBusterModule.h>

#include <framework/logging/LogMethod.h>

using namespace Belle2;

REG_MODULE(EventsOfDoomBuster)

EventsOfDoomBusterModule::EventsOfDoomBusterModule()
{
  setDescription(R"DOC(
Module that flags an event destined for doom at reconstruction,
* based on the size of selected hits/digits containers after the unpacking.
* This is meant to be registered in the path *after* the unpacking, but *before* reconstruction.
)DOC");

  addParam("nCDCHitsMax", m_nCDCHitsMax, 
R"DOC(the max number of CDC hits for an event to be kept for reconstruction. 
By default, no events are skipped based upon this requirement.)DOC", m_nCDCHitsMax);
  addParam("nSVDShaperDigitsMax", m_nSVDShaperDigitsMax,
           R"DOC(the max number of SVD shaper digits for an event to be kept for reconstruction.
By default, no events are skipped based upon this requirement.)DOC", m_nSVDShaperDigitsMax);

  setPropertyFlags(c_ParallelProcessingCertified);
}

EventsOfDoomBusterModule::~EventsOfDoomBusterModule() = default;

void EventsOfDoomBusterModule::initialize()
{
  m_eventInfo.isRequired();
  m_cdcHits.isRequired();
  m_svdShaperDigits.isRequired();
}


void EventsOfDoomBusterModule::event()
{
  const unsigned int nCDCHits = m_cdcHits.getEntries();
  const unsigned int nSVDShaperDigits = m_svdShaperDigits.getEntries();

  B2DEBUG(20, "Event: " << m_eventInfo->getEvent() << " - nCDCHits: " << nCDCHits << ", nSVDShaperDigits: " << nSVDShaperDigits);

  const bool doomCDC = nCDCHits > m_nCDCHitsMax;
  const bool doomSVD = nSVDShaperDigits > m_nSVDShaperDigitsMax;

  if (doomCDC) {
    B2WARNING("Skip event --> Too much occupancy for reco!" <<
              LogVar("event", m_eventInfo->getEvent()) <<
              LogVar("run", m_eventInfo->getRun()) <<
              LogVar("exp", m_eventInfo->getExperiment()) <<
              LogVar("nCDCHits", nCDCHits) <<
              LogVar("nCDCHitsMax", m_nCDCHitsMax));
  }

  if (doomSVD) {
    B2WARNING("Skip event --> Too much occupancy for reco!" <<
              LogVar("event", m_eventInfo->getEvent()) <<
              LogVar("run", m_eventInfo->getRun()) <<
              LogVar("exp", m_eventInfo->getExperiment()) <<
              LogVar("nSVDShaperDigits", nSVDShaperDigits) <<
              LogVar("nSVDShaperDigitsMax", m_nSVDShaperDigitsMax));
  }

  setReturnValue(doomCDC or doomSVD);
}
