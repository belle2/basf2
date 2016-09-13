/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class TMVAHitGapSegmentPairFilter : public TMVAFilter<SkimmedHitGapSegmentPairVarSet> {

    private:
      /// Type of the base class
      typedef TMVAFilter<SkimmedHitGapSegmentPairVarSet> Super;

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVAHitGapSegmentPairFilter()
        : Super("HitGapSegmentPairFilter", 0.01)
      {
      }

    }; // end class TMVAHitGapSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
