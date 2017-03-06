/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
