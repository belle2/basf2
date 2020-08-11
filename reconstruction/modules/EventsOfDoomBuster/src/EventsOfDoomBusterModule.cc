/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <reconstruction/modules/EventsOfDoomBuster/EventsOfDoomBusterModule.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

REG_MODULE(EventsOfDoomBuster)

EventsOfDoomBusterModule::EventsOfDoomBusterModule()
{
  setDescription(R"DOC(
Module that flags an event destined for doom at reconstruction,
* based on the size of selected hits/digits containers after the unpacking.
* This is meant to be registered in the path *after* the unpacking, but *before* reconstruction.
)DOC");

  setPropertyFlags(c_ParallelProcessingCertified);
}

EventsOfDoomBusterModule::~EventsOfDoomBusterModule() = default;

void EventsOfDoomBusterModule::initialize()
{
  m_eventInfo.isRequired();
  m_cdcHits.isOptional();
  m_svdShaperDigits.isOptional();
}

void EventsOfDoomBusterModule::beginRun()
{
  if (!m_eventsOfDoomParameters.isValid())
    B2FATAL("EventsOfDoom parameters are not available.");
  m_nCDCHitsMax = m_eventsOfDoomParameters->getNCDCHitsMax();
  m_nSVDShaperDigitsMax = m_eventsOfDoomParameters->getNSVDShaperDigitsMax();
}

void EventsOfDoomBusterModule::event()
{
  const uint32_t nCDCHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
  const uint32_t nSVDShaperDigits = m_svdShaperDigits.isOptional() ? m_svdShaperDigits.getEntries() : 0;

  B2DEBUG(20, "Event: " << m_eventInfo->getEvent() << " - nCDCHits: " << nCDCHits << ", nSVDShaperDigits: " << nSVDShaperDigits);

  const bool doomCDC = nCDCHits > m_nCDCHitsMax;
  const bool doomSVD = nSVDShaperDigits > m_nSVDShaperDigitsMax;

  if (doomCDC) {
    B2ERROR("Skip event --> Too much occupancy from CDC for reconstruction!" <<
            LogVar("event", m_eventInfo->getEvent()) <<
            LogVar("run", m_eventInfo->getRun()) <<
            LogVar("exp", m_eventInfo->getExperiment()) <<
            LogVar("nCDCHits", nCDCHits) <<
            LogVar("nCDCHitsMax", m_nCDCHitsMax));
  }

  if (doomSVD) {
    B2ERROR("Skip event --> Too much occupancy from SVD for reconstruction!" <<
            LogVar("event", m_eventInfo->getEvent()) <<
            LogVar("run", m_eventInfo->getRun()) <<
            LogVar("exp", m_eventInfo->getExperiment()) <<
            LogVar("nSVDShaperDigits", nSVDShaperDigits) <<
            LogVar("nSVDShaperDigitsMax", m_nSVDShaperDigitsMax));
  }

  setReturnValue(doomCDC or doomSVD);
}
