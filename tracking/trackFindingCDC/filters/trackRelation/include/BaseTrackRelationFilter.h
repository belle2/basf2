/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
