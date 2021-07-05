/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of axial stereo segment pairs based on simple criteria.
    class SimpleSegmentPairRelationFilter : public BaseSegmentPairRelationFilter {

    public:
      /// Main filter method weighting the relation between the segment pairs.
      Weight operator()(const CDCSegmentPair& fromSegmentPair, const CDCSegmentPair& toSegmentPair) final;
    };
  }
}
