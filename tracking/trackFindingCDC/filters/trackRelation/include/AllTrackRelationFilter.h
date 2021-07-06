/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Relation filter that lets all possible combinations pass.
    class AllTrackRelationFilter : public BaseTrackRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseTrackRelationFilter;

    public:
      /// Implementation accepting all possible neighbors.
      Weight operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack) final;
    };
  }
}
