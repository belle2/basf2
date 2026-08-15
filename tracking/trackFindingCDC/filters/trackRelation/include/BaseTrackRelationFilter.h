/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/RelationFilter.dcl.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::RelationFilter<const TrackingUtilities::CDCTrack>;

    /// Base class for filtering the neighborhood of tracks
    class BaseTrackRelationFilter : public TrackingUtilities::RelationFilter<const TrackingUtilities::CDCTrack> {

    public:
      /// Default constructor
      BaseTrackRelationFilter();

      /// Default destructor
      ~BaseTrackRelationFilter();

      /// Reinforce that the full range of tracks is possible as no particular default is applicable.
      std::vector<const TrackingUtilities::CDCTrack*> getPossibleTos(
        const TrackingUtilities::CDCTrack* from,
        const std::vector<const TrackingUtilities::CDCTrack*>& tracks) const final;
    };
  }
}
