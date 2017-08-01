/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/states/CKFResultObject.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /// Findlet for marking all wire hits in a given result vector as used
  template <class ASeedObject, class AHitObject>
  class WireHitTagger : public TrackFindingCDC::Findlet<const CKFResultObject<ASeedObject, AHitObject>> {
  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "markUsedWireHits"), m_param_markUsedWireHits,
                                    "Mark used wire hits as assigned.",
                                    m_param_markUsedWireHits);
    }

    /// Mark all space points as used, that they share clusters if the given kind with the results.
    void apply(const std::vector<CKFResultObject<ASeedObject, AHitObject>>& results) override
    {
      if (not m_param_markUsedWireHits) {
        return;
      }

      for (const auto& result : results) {
        const auto& hits = result.getHits();

        for (const auto& hit : hits) {
          hit->getWireHit().getAutomatonCell().setTakenFlag();
        }
      }
    }

  private:
    /// Parameter: Mark used wire hits as assigned
    bool m_param_markUsedWireHits = true;
  };
}
