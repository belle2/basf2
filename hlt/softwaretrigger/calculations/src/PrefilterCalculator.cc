/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <hlt/softwaretrigger/calculations/PrefilterCalculator.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace SoftwareTrigger;

void PrefilterCalculator::requireStoreArrays()
{
  m_l1Trigger.isOptional();
}

PrefilterCalculator::PrefilterCalculator()
{

}

void PrefilterCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
{
  //..Filters lines for HLTPrefilter decision
  calculationResult["EventsOfDoom"] = 0; /**< Events with high SVD-CDC occupancy */
  calculationResult["InjectionStrip"] = 0; /**< Events in the injection strip */

  // Initialise parameters of EventsOfDoomBuster
  if (!m_eventsOfDoomParameters.isValid())
    B2FATAL("EventsOfDoom parameters are not available.");

  // EventsOfDoomBuster mode thresholds
  m_eodbPrefilter.nCDCHitsMax = m_eventsOfDoomParameters->getNCDCHitsMax();
  m_eodbPrefilter.nSVDShaperDigitsMax = m_eventsOfDoomParameters->getNSVDShaperDigitsMax();

  // EventsOfDoomBuster mode decision
  if (m_eodbPrefilter.computeDecision()) {
    calculationResult["EventsOfDoom"] = 1;
  }

  // Initialise parameters of HLTPrefilter
  if (!m_hltPrefilterParameters.isValid())
    B2FATAL("HLTprefilter parameters are not available.");

  // Injection Timing mode thresholds
  m_timingPrefilter.LERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMin();
  m_timingPrefilter.LERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getLERtimeSinceLastInjectionMax();
  m_timingPrefilter.HERtimeSinceLastInjectionMin = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMin();
  m_timingPrefilter.HERtimeSinceLastInjectionMax = m_hltPrefilterParameters->getHERtimeSinceLastInjectionMax();
  m_timingPrefilter.LERtimeInBeamCycleMin        = m_hltPrefilterParameters->getLERtimeInBeamCycleMin();
  m_timingPrefilter.LERtimeInBeamCycleMax        = m_hltPrefilterParameters->getLERtimeInBeamCycleMax();
  m_timingPrefilter.HERtimeInBeamCycleMin        = m_hltPrefilterParameters->getHERtimeInBeamCycleMin();
  m_timingPrefilter.HERtimeInBeamCycleMax        = m_hltPrefilterParameters->getHERtimeInBeamCycleMax();
  m_timingPrefilter.prescale = m_hltPrefilterParameters->getHLTPrefilterPrescale();

  //find out if we are in the passive veto (i=0) or in the active veto window (i=1)
  int index = 0; //events accepted in the passive veto window but not in the active
  try {
    if (m_l1Trigger->testInput("passive_veto") == 1 &&  m_l1Trigger->testInput("cdcecl_veto") == 0) index = 1; //events in active veto
  } catch (const std::exception&) {
  }

  // Check if event is in active veto
  if (index == 1) {

    // Injection time mode decision
    if (m_timingPrefilter.computeDecision()) {
      calculationResult["InjectionStrip"] = 1;
    }

  }


}
