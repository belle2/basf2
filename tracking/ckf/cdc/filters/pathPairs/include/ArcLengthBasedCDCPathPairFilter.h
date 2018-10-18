/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  class ArcLengthBasedCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final {
      const auto& lhs = *pair.first;
      const auto& rhs = *pair.second;
      const auto& lhsLastState = lhs.back();
      const auto& rhsLastState = rhs.back();

      const auto lhsArcLength = lhsLastState.getArcLength();
      const auto rhsArcLength = rhsLastState.getArcLength();

      // priority is most hits
      if (lhs.size() != rhs.size())
      {
        return lhs.size() > rhs.size();
      }

      return lhsArcLength < rhsArcLength;
    }
  };
}
