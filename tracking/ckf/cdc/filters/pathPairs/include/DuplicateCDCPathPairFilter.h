/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Simon Kurz                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <utility>

namespace Belle2 {
  /// Simple filter to distinguish between photon conversion and Bremsstrahlung
  class DuplicateCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if too many duplicate hits found
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;

      const CDCCKFPath* shortPath = &lhs;
      const CDCCKFPath* longPath = &rhs;
      if (longPath->size() < shortPath->size())
      {
        std::swap(shortPath, longPath);
      }

      int match = 0;
      int total = shortPath->size() - 1;
      for (const auto& sState : *shortPath)
      {
        if (sState.isSeed()) {
          continue;
        }
        if (std::any_of(longPath->begin(), longPath->end(), [&sState](auto & lState)
        {return !lState.isSeed() && sState.getWireHit() == lState.getWireHit();})) {
          match++;
        }
      }

      return double(match) / double(total) > m_minFractionSharedHits;
    }

    /// Expose the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minFractionSharedHits"),
                                    m_minFractionSharedHits,
                                    "Fraction of shared hits to distinguish photon conversion/Bremsstahlung",
                                    m_minFractionSharedHits);
    }

  private:
    /// minimal fraction of shared hits
    double m_minFractionSharedHits = 0.9;
  };
}

