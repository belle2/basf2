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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/ca/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
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

    /// Class vehicle to transport the variable names
    struct HitGapTrackRelationVarNames : public VarNames<Relation<const CDCTrack> > {

      /// Number of variables to be generated
      static const size_t nVars = size(hitGapTrackRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return hitGapTrackRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class HitGapTrackRelationVarSet : public VarSet<HitGapTrackRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCTrack>* ptrTrackRelation) final;
    };
  }
}
