/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

WireHitBackgroundDetector::WireHitBackgroundDetector()
{
  /// adding the filter as a subordinary processing signal listener.
  this->addProcessingSignalListener(&m_wireHitFilter);
}

std::string WireHitBackgroundDetector::getDescription()
{
  return "Marks hits as background based on the result of a filter.";
}

void WireHitBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  m_wireHitFilter.exposeParameters(moduleParamList, prefix);
}

void WireHitBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  // we will apply classifier to all hits at once, this is much faster for python-based MVA
  std::vector<CDCWireHit*> wireHitsPtr(wireHits.size());
  for (size_t iHit = 0;  iHit < wireHits.size(); iHit += 1) {
    wireHitsPtr[iHit] = &wireHits[iHit];
  }

  auto probs = m_wireHitFilter(wireHitsPtr);

  for (size_t iHit = 0; iHit < probs.size(); iHit += 1) {
    if (std::isnan(probs[iHit])) {
      wireHits[iHit]->setTakenFlag();
      wireHits[iHit]->setBackgroundFlag();
      wireHits[iHit]->setBadADCOrTOTFlag();
    }
  }
}
