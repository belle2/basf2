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
#include <framework/dbobjects/BunchStructure.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <cdc/dataobjects/CDCHit.h>
#include <ecl/dataobjects/ECLDigit.h>

using namespace Belle2;

REG_MODULE(HLTprefilter);

// -------------------- TimingCutHelper --------------------
bool TimingCutMode::computeDecision(StoreObjPtr<EventLevelTriggerTimeInfo>* ttd,
                                    DBObjPtr<BunchStructure>* bunch,
                                    DBObjPtr<HardwareClockSettings>* clock) const
{
  if (!ttd || !bunch || !clock) return false;

  double c_revolutionTime = bunch->getRFBucketsPerRevolution() / (clock->getAcceleratorRF() * 1e3);          // [microsecond]
  double c_globalClock = clock->getGlobalClockFrequency() * 1e3;                                             // [microsecond]

  double timeSinceLastInj = ttd->getTimeSinceLastInjection() / c_globalClock;                                // [microsecond]
  double timeInBeamCycle = timeSinceLastInj - (int)(timeSinceLastInj / c_revolutionTime) * c_revolutionTime; // [microsecond]

  bool LER_strip = (m_timingPrefilter.LERtimeSinceLastInjectionMin < timeSinceLastInj &&
                    timeSinceLastInj < m_timingPrefilter.LERtimeSinceLastInjectionMax &&
                    m_timingPrefilter.LERtimeInBeamCycleMin < timeInBeamCycle &&
                    timeInBeamCycle < m_timingPrefilter.LERtimeInBeamCycleMax);

  bool HER_strip = (m_timingPrefilter.HERtimeSinceLastInjectionMin < timeSinceLastInj &&
                    timeSinceLastInj < m_timingPrefilter.HERtimeSinceLastInjectionMax &&
                    m_timingPrefilter.HERtimeInBeamCycleMin < timeInBeamCycle &&
                    timeInBeamCycle < m_timingPrefilter.HERtimeInBeamCycleMax);

  return (LER_strip || HER_strip) &&
         !Belle2::SoftwareTrigger::makePreScale(prescale);
}

// -------------------- CdcEclCutHelper --------------------
bool CdcEclCutMode::computeDecision() const
{
  const uint32_t NcdcHits = m_cdcHits ? m_cdcHits->getEntries() : 0;
  const uint32_t NeclDigits = m_eclDigits ? m_eclDigits->getEntries() : 0;

  return (NcdcHits > m_cdceclPrefilter.nCDCHitsMax && NeclDigits > m_cdceclPrefilter.nECLDigitsMax) &&
         !Belle2::SoftwareTrigger::makePreScale(prescale);
}

// -------------------- HLTprefilterModule --------------------
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
    if (!m_hltPrefilterParameters.isValid())
        B2FATAL("HLTprefilter parameters are not available.");

    // Timing mode thresholds
    m_timingPrefilter.LERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMin();
    m_timingPrefilter.LERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMax();
    m_timingPrefilter.HERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMin();
    m_timingPrefilter.HERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMax();
    m_timingPrefilter.LERtimeInBeamCycleMin        = m_hltPrefilterParameters->getLERtimeInBeamCycleMin();
    m_timingPrefilter.LERtimeInBeamCycleMax        = m_hltPrefilterParameters->getLERtimeInBeamCycleMax();
    m_timingPrefilter.HERtimeInBeamCycleMin        = m_hltPrefilterParameters->getHERtimeInBeamCycleMin();
    m_timingPrefilter.HERtimeInBeamCycleMax        = m_hltPrefilterParameters->getHERtimeInBeamCycleMax();
    m_timingPrefilter.prescale = m_hltPrefilterParameters->getHLTprefilterPrescale();

    // CDC-ECL mode thresholds
    m_cdceclPrefilter.nCDCHitsMax = m_hltPrefilterParameters->getCDCHitsMax();
    m_cdceclPrefilter.nECLDigitsMax = m_hltPrefilterParameters->getECLDigitsMax();
    m_cdceclPrefilter.prescale = m_hltPrefilterParameters->getHLTprefilterPrescale();

    m_HLTprefilterMode = static_cast<HLTPrefilterMode>(m_hltPrefilterParameters->getHLTprefilterMode());
}

void HLTprefilterModule::event()
{
    int index = 0;
    try {
        if (m_trgSummary->testInput("passive_veto") == 1 &&
            m_trgSummary->testInput("cdcecl_veto") == 0)
            index = 1;
    } catch (const std::exception&) {}

    m_decisions.clear();

    if (index == 1) {
        m_decisions[TimingCut] = m_timingPrefilter.computeDecision(m_TTDInfo, m_bunchStructure, m_clockSettings);
        m_decisions[CdcEclCut] = m_cdceclPrefilter.computeDecision();
    }

    // Logging
    if (m_decisions[TimingCut])
        B2ERROR("Skip event --> HLTprefilter tagged this event to be from injection strips" <<
                LogVar("event", m_eventInfo->getEvent()) <<
                LogVar("run", m_eventInfo->getRun()) <<
                LogVar("exp", m_eventInfo->getExperiment()));

    if (m_decisions[CdcEclCut])
        B2ERROR("Skip event --> HLTprefilter tagged this event with high CDC-ECL occupancy" <<
                LogVar("event", m_eventInfo->getEvent()) <<
                LogVar("run", m_eventInfo->getRun()) <<
                LogVar("exp", m_eventInfo->getExperiment()));

    // Return only the selected mode
    if (m_HLTprefilterMode == TimingCut)
        setReturnValue(m_decisions[TimingCut]);
    else
        setReturnValue(m_decisions[CdcEclCut]);
}

