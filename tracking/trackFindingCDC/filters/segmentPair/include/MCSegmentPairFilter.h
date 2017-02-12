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

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to stereo segment pairs based on MC information.
    class MCSegmentPairFilter : public MCSymmetricFilterMixin<BaseSegmentPairFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetricFilterMixin<BaseSegmentPairFilter>;

    public:
      /// Constructor
      explicit MCSegmentPairFilter(bool allowReverse = true);

      /// Checks if a axial stereo segment pair is a good combination.
      Weight operator()(const CDCSegmentPair& segmentPair) final;
    };
  }
}
