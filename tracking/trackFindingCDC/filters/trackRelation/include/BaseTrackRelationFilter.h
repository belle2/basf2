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

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    // Guard to prevent repeated instantiations
    extern template class RelationFilter<const CDCTrack>;

    /// Base class for filtering the neighborhood of tracks
    class BaseTrackRelationFilter : public RelationFilter<const CDCTrack> {

    public:
      /// Default constructor
      BaseTrackRelationFilter();

      /// Default destructor
      ~BaseTrackRelationFilter();

      /// Reenforce that the full range of tracks is possible as no particular default is applicable.
      std::vector<const CDCTrack*> getPossibleTos(
        const CDCTrack* from,
        const std::vector<const CDCTrack*>& tracks) const final;
    };
  }
}
