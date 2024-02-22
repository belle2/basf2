/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/dataobjects/CDCHit.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

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

  moduleParamList->addParameter(prefixed(prefix, "MVAcut"),
                                m_mvaCutValue,
                                "The cut value of the mva output below which the object is rejected",
                                m_mvaCutValue);
}


void WireHitBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  if (m_mvaCutValue == 0) {
    for (CDCWireHit& wireHit : wireHits) {

      Weight wireHitWeight = m_wireHitFilter(wireHit);

      if (std::isnan(wireHitWeight)) {
        wireHit->setBackgroundFlag();
        wireHit->setTakenFlag();
        wireHit->setBadADCOrTOTFlag();
      }
    }
  }
  // we will apply MVA classifier to all hits
  else {
    int nHits = wireHits.size();
    int nFeature = 4;
    auto X = std::unique_ptr<float[]>(new float[nHits * nFeature]);
    size_t iHit = 0;
    for (CDCWireHit& wireHit : wireHits) {
      const auto* cdcHit = wireHit.getHit();
      X[nFeature * iHit + 0] = cdcHit->getADCCount();
      X[nFeature * iHit + 1] = cdcHit->getTOT();
      X[nFeature * iHit + 2] = cdcHit->getTDCCount();
      X[nFeature * iHit + 3] = wireHit.getISuperLayer() == 0 ? 0 : 1; // Layer?
      iHit++;
    }
    // evaluate:
    auto probs = m_wireHitFilter.predict(X.get(), nFeature, nHits);
    for (iHit = 0; iHit < probs.size(); iHit++) {
      if (probs[iHit] < m_mvaCutValue) {
        wireHits[iHit]->setTakenFlag();
        wireHits[iHit]->setBackgroundFlag();
        wireHits[iHit]->setBadADCOrTOTFlag();
      }
    }
  }
}
