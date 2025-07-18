/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/modules/HLTprefilter/HLTprefilterModule.h>

#include <framework/logging/Logger.h>

#include <iostream>
using namespace Belle2;

REG_MODULE(HLTprefilter);

HLTprefilterModule::HLTprefilterModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(
This module filters the injection background based on predefined selections.
* This is meant to be registered in the path *after* the unpacking, but *before* HLT processing.
)DOC");

  setPropertyFlags(c_ParallelProcessingCertified);
}

HLTprefilterModule::~HLTprefilterModule() = default;

void HLTprefilterModule::initialize()
{
  m_eventInfo.isRequired();
  m_trgSummary.isOptional();
  m_TTDInfo.isOptional();
  m_cdcHits.isOptional();
  m_eclDigits.isOptional();
}

void HLTprefilterModule::beginRun()
{
/*	
  if (!m_hltPrefilterParameters.isValid())
    B2FATAL("HLTprefilter parameters are not available.");
  m_LERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMin();
  m_LERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMax();
  m_HERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMin();
  m_HERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMax();
  m_LERtimeInBeamCycleMin = m_hltPrefilterParameters->getLERtimeInBeamCycleMin();
  m_LERtimeInBeamCycleMax = m_hltPrefilterParameters->getLERtimeInBeamCycleMax();
  m_HERtimeInBeamCycleMin = m_hltPrefilterParameters->getHERtimeInBeamCycleMin();
  m_HERtimeInBeamCycleMax = m_hltPrefilterParameters->getHERtimeInBeamCycleMax();

  m_cdcHitsMax = m_hltPrefilterParameters->getCDCHitsMax();
  m_eclDigitsMax = m_hltPrefilterParameters->getECLDigitsMax();

  m_HLTprefilterMode = m_hltPrefilterParameters->getHLTprefilterMode();

  m_HLTprefilterPrescale = m_hltPrefilterParameters->getHLTprefilterPrescale();


*/
}

void HLTprefilterModule::event()
{

  //find out if we are in the passive veto (i=0) or in the active veto window (i=1)
  int index = 0; //events accepted in the passive veto window but not in the active
  try {
    if (m_trgSummary->testInput("passive_veto") == 1 &&  m_trgSummary->testInput("cdcecl_veto") == 0) index = 1; //events in active veto
  } catch (const std::exception&) {
  }

  injection_strip = false;
  cdcecl_threshold = false;

  // Tag events from active window
  if (index == 1) {

    if (m_TTDInfo.isValid()) {

      double c_revolutionTime = m_bunchStructure->getRFBucketsPerRevolution() / (m_clockSettings->getAcceleratorRF() * 1e3);
      double c_globalClock = m_clockSettings->getGlobalClockFrequency() * 1e3;

      double timeSinceLastInj = m_TTDInfo->getTimeSinceLastInjection() / c_globalClock;
      double timeInBeamCycle = timeSinceLastInj - (int)(timeSinceLastInj / c_revolutionTime) * c_revolutionTime;

      bool LER_strip = (m_LERtimeSinceLastInjectionMin < timeSinceLastInj && timeSinceLastInj < m_LERtimeSinceLastInjectionMax
                        && m_LERtimeInBeamCycleMin < timeInBeamCycle && timeInBeamCycle < m_LERtimeInBeamCycleMax);
      bool HER_strip = (m_HERtimeSinceLastInjectionMin < timeSinceLastInj && timeSinceLastInj < m_HERtimeSinceLastInjectionMax
                        && m_HERtimeInBeamCycleMin < timeInBeamCycle && timeInBeamCycle < m_HERtimeInBeamCycleMax);

      if ((LER_strip || HER_strip) && !Belle2::SoftwareTrigger::makePreScale(m_HLTprefilterPrescale)) 
	      injection_strip = true;
    }
  
    const uint32_t NcdcHits = m_cdcHits.isOptional() ? m_cdcHits.getEntries() : 0;
    const uint32_t NeclDigits = m_eclDigits.isOptional() ? m_eclDigits.getEntries() : 0;
    if (NcdcHits > m_cdcHitsMax && NeclDigits > m_eclDigitsMax && !Belle2::SoftwareTrigger::makePreScale(m_HLTprefilterPrescale)) 
          cdcecl_threshold = true;

  }

  if (injection_strip) {
    B2ERROR("Skip event --> HLTprefilter tagged this event to be from injection strips" <<
            LogVar("event", m_eventInfo->getEvent()) <<
            LogVar("run", m_eventInfo->getRun()) <<
            LogVar("exp", m_eventInfo->getExperiment()));
  }

  if (cdcecl_threshold) {
    B2ERROR("Skip event --> HLTprefilter tagged this event with high CDC-ECL occupancy" <<
            LogVar("event", m_eventInfo->getEvent()) <<
            LogVar("run", m_eventInfo->getRun()) <<
            LogVar("exp", m_eventInfo->getExperiment()));
  }

  if (m_HLTprefilterMode == 0)
    setReturnValue(injection_strip);
  else
    setReturnValue(cdcecl_threshold);



}


