/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/modules/HLTprefilter/HLTprefilterModule.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

REG_MODULE(HLTprefilter);

HLTprefilterModule::HLTprefilterModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Prefilter module to suppress the injection background)DOC");

  setPropertyFlags(c_ParallelProcessingCertified);
}

HLTprefilterModule::~HLTprefilterModule() = default;

void HLTprefilterModule::initialize()
{
  m_eventInfo.isRequired();
  m_trgSummary.isOptional();
  m_TTDInfo.isOptional();
}

void HLTprefilterModule::beginRun()
{
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

  // Tag events from active window
  if (index == 1) {
    if (m_TTDInfo->isValid()) {
      double timeSinceLastInj = m_TTDInfo->getTimeSinceLastInjection() / m_globalClock;
      double timeInBeamCycle = timeSinceLastInj - (int)(timeSinceLastInj / m_revolutionTime) * m_revolutionTime;

      bool LER_strip = (m_LERtimeSinceLastInjectionMin < timeSinceLastInj && timeSinceLastInj < m_LERtimeSinceLastInjectionMax
                        && m_LERtimeInBeamCycleMin < timeInBeamCycle && timeInBeamCycle < m_LERtimeInBeamCycleMax);
      bool HER_strip = (m_HERtimeSinceLastInjectionMin < timeSinceLastInj && timeSinceLastInj < m_HERtimeSinceLastInjectionMax
                        && m_HERtimeInBeamCycleMin < timeInBeamCycle && timeInBeamCycle < m_HERtimeInBeamCycleMax);
      bool timing_window = LER_strip || HER_strip;

      if (timing_window) {
        injection_strip = true;
      }
    }
  }

  if (injection_strip == true) {
    B2ERROR("Skip event --> Removing injection strips as prefilter" <<
            LogVar("event", m_eventInfo->getEvent()) <<
            LogVar("run", m_eventInfo->getRun()) <<
            LogVar("exp", m_eventInfo->getExperiment()));
  }

  setReturnValue(injection_strip);

}



