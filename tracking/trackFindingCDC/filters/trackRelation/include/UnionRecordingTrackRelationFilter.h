/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/trackRelation/TrackRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered relations between tracks.
    class UnionRecordingTrackRelationFilter:
      public UnionRecordingFilter<TrackRelationFilterFactory> {

    private:
      /// Type of the base class
      typedef  UnionRecordingFilter<TrackRelationFilterFactory> Super;

    public:
      /// Valid names of variable sets for tracks.
      virtual std::vector<std::string> getValidVarSetNames() const override;

      /// Create a concrete variables set for tracks from a name.
      virtual
      std::unique_ptr<BaseVarSet<Relation<const CDCTrack> > >
      createVarSet(const std::string& name) const override;
    };
  }
}
