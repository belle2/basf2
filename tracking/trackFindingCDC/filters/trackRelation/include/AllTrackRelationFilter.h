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
      typedef BaseTrackRelationFilter Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Implementation accepting all possible neighbors.
      virtual Weight operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack) override final;

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
