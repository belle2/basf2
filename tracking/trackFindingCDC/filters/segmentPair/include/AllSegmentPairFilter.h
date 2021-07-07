/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Filter for the constuction of axial to stereo segment pairs based on simple criteria.
    class AllSegmentPairFilter : public Filter<CDCSegmentPair> {

    public:
      /**
       *  Checks if a pair of segments is a good combination.
       *  All implementation always accepts with the total number of hits as weight.
       */
      Weight operator()(const CDCSegmentPair& segmentPair) final;
    };
  }
}
