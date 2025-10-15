/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/modules/HLTPrefilter/HLTPrefilterModule.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <mdst/dataobjects/TRGSummary.h>

using namespace Belle2;

REG_MODULE(HLTPrefilter);

// -------------------- HLTPrefilterModule --------------------
HLTPrefilterModule::HLTPrefilterModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(
This module filters the injection background based on predefined selections.
* This is meant to be registered in the path *after* the unpacking, but *before* HLT processing.
)DOC");

  setPropertyFlags(c_ParallelProcessingCertified);
}

HLTPrefilterModule::~HLTPrefilterModule() = default;

void HLTPrefilterModule::initialize()
{
    m_eventInfo.isRequired();
    m_trgSummary.isOptional();
}

void HLTPrefilterModule::beginRun()
{
    if (!m_hltPrefilterParameters.isValid())
        B2FATAL("HLTPrefilter parameters are not available.");

    // Parameters for HLTPrefilter in timing state
    m_timingPrefilter.LERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMin();
    m_timingPrefilter.LERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMax();
    m_timingPrefilter.HERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMin();
    m_timingPrefilter.HERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMax();
    m_timingPrefilter.LERtimeInBeamCycleMin        = m_hltPrefilterParameters->getLERtimeInBeamCycleMin();
    m_timingPrefilter.LERtimeInBeamCycleMax        = m_hltPrefilterParameters->getLERtimeInBeamCycleMax();
    m_timingPrefilter.HERtimeInBeamCycleMin        = m_hltPrefilterParameters->getHERtimeInBeamCycleMin();
    m_timingPrefilter.HERtimeInBeamCycleMax        = m_hltPrefilterParameters->getHERtimeInBeamCycleMax();
    m_timingPrefilter.prescale = m_hltPrefilterParameters->getHLTPrefilterPrescale();

    // Parameters for HLTPrefilter in CDCECL state
    m_cdceclPrefilter.nCDCHitsMax = m_hltPrefilterParameters->getCDCHitsMax();
    m_cdceclPrefilter.nECLDigitsMax = m_hltPrefilterParameters->getECLDigitsMax();
    m_cdceclPrefilter.prescale = m_hltPrefilterParameters->getHLTPrefilterPrescale();

    // Get prefilter state (cut on timing/CDC-ECL occupancy) for operation
    m_HLTPrefilterState = static_cast<HLTPrefilterState>(m_hltPrefilterParameters->getHLTPrefilterState());

}

void HLTPrefilterModule::event()
{
    int index = 0;
    try {
        if (m_trgSummary->testInput("passive_veto") == 1 &&
            m_trgSummary->testInput("cdcecl_veto") == 0)
            index = 1;
    } catch (const std::exception&) {}

    m_decisions.clear();

    if (index == 1) {

    /// Compute prefilter decision with timing cuts
    m_decisions[TimingCut] = m_timingPrefilter.computeDecision();

    /// Compute prefilter decision with CDC-ECL occupancy
    m_decisions[CDCECLCut] = m_cdceclPrefilter.computeDecision();

    }

    // Logging
    if (m_decisions[TimingCut])
        B2ERROR("Skip event --> HLTPrefilter tagged this event to be from injection strips" <<
                LogVar("event", m_eventInfo->getEvent()) <<
                LogVar("run", m_eventInfo->getRun()) <<
                LogVar("exp", m_eventInfo->getExperiment()));

    if (m_decisions[CDCECLCut])
        B2ERROR("Skip event --> HLTPrefilter tagged this event with high CDC-ECL occupancy" <<
                LogVar("event", m_eventInfo->getEvent()) <<
                LogVar("run", m_eventInfo->getRun()) <<
                LogVar("exp", m_eventInfo->getExperiment()));

    // Return only the prefilter condition set by the parameter
    if (m_HLTPrefilterState == TimingCut)
        setReturnValue(m_decisions[TimingCut]);
    else
        setReturnValue(m_decisions[CDCECLCut]);
}

