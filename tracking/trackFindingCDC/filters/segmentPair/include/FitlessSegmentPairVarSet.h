/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitlessSegmentPairVarNames[] = {
      "from_ndf",
      "to_ndf",

      "from_chi2_over_ndf",
      "to_chi2_over_ndf",

      "from_p_value",
      "to_p_value",

      "abs_avg_curv",
      "delta_curv",
      "delta_curv_var",
      "delta_curv_pull",

      "delta_pos_phi",
      "delta_mom_phi",
      "from_delta_mom_phi",
      "to_delta_mom_phi",
      "delta_alpha",

      "reco_arc_length_gap",
      "stereo_arc_length",
      "near_reco_z",
      "near_z_bound_factor",
      "far_reco_z",
      "far_z_bound_factor",

      "coarse_tanl",
      "stereo_rel_size",

      "arc_length_front_offset",
      "arc_length_back_offset",
      "from_arc_length_total",
      "to_arc_length_total",
      "arc_length_gap",
    };

    /// Vehicle class to transport the variable names
    struct FitlessSegmentPairVarNames : public VarNames<CDCSegmentPair> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitlessSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitlessSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitlessSegmentPairVarSet : public VarSet<FitlessSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCSegmentPair* ptrSegmentPair) final;
    };
  }
}
