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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    constexpr
    static char const* const hitGapTrackRelationVarNames[] = {
      "delta_hit_pos_phi",
      "delta_hit_mom_phi",
      "delta_hit_alpha",

      "delta_hit_z",
      "hit_distance",
      "hit_long_distance",
      "delta_hit_distance",

      "from_hit_forward",
      "to_hit_forward",
      "hit_forward",
      "hit_ptolemy",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a track
     */
    class HitGapTrackRelationVarNames : public VarNames<Relation<const CDCTrack> > {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(hitGapTrackRelationVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return hitGapTrackRelationVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class HitGapTrackRelationVarSet : public VarSet<HitGapTrackRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<HitGapTrackRelationVarNames>;

    public:
      /// Construct the varset.
      explicit HitGapTrackRelationVarSet();

      /// Generate and assign the variables from the track pair
      bool extract(const Relation<const CDCTrack>* ptrTrackRelation) override;
    };
  }
}
