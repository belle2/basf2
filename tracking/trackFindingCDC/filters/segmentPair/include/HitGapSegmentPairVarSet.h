/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/varsets/VarSet.h>
#include <tracking/trackingUtilities/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCSegmentPair;
  }
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
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

    /// Vehicle class to transport the variable names
    struct HitGapSegmentPairVarNames : public TrackingUtilities::VarNames<TrackingUtilities::CDCSegmentPair> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = TrackingUtilities::size(hitGapSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return hitGapSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class HitGapSegmentPairVarSet : public TrackingUtilities::VarSet<HitGapSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const TrackingUtilities::CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
