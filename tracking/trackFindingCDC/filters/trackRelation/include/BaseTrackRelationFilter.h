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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCTrack> >;

    /// Base class for filtering the neighborhood of tracks
    class BaseTrackRelationFilter : public Filter<Relation<const CDCTrack> > {

    public:
      /// Returns the full range of tracks.
      std::vector<const CDCTrack*> getPossibleNeighbors(
        const CDCTrack* track,
        const std::vector<const CDCTrack*>::const_iterator& itBegin,
        const std::vector<const CDCTrack*>::const_iterator& itEnd) const;

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all track neighbors.
       */
      virtual Weight operator()(const CDCTrack& from, const CDCTrack& to);

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCTrack>& relation) override;
    };
  }
}
