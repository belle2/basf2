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
}

PrefilterCalculator::PrefilterCalculator()
{
}

void PrefilterCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
{
  //..Filters lines for HLTPrefilter decision
  calculationResult["EventsOfDoom"] = 0; /**< Events with high SVD-CDC occupancy */
  calculationResult["InjectionStrip"] = 0; /**< Events in the injection strip */

  // EventsOfDoomBuster mode decision
  if (m_eodbPrefilter.computeDecision()) {
    calculationResult["EventsOfDoom"] = 1;
  }
  // Injection time mode decision
  if (m_timingPrefilter.computeDecision()) {
    calculationResult["InjectionStrip"] = 1;
  }

}
