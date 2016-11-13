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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const hitGapSegmentPairVarNames[] = {
      "delta_hit_pos_phi",
      "delta_hit_mom_phi",
      "delta_hit_alpha",

      "hit_distance",
      "hit_long_distance",
      "delta_hit_distance",

      "from_hit_forward",
      "to_hit_forward",
      "hit_forward",

      "hit_reco_z",
      "hit_z_bound_factor",
      "hit_arclength_gap",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class HitGapSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(hitGapSegmentPairVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return hitGapSegmentPairVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class HitGapSegmentPairVarSet : public VarSet<HitGapSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<HitGapSegmentPairVarNames>;

    public:
      /// Construct the varset.
      explicit HitGapSegmentPairVarSet();

      /// Generate and assign the variables from the segment pair
      bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
