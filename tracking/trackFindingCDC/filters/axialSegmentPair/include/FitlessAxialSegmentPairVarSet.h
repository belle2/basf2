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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCAxialSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitlessAxialSegmentPairVarNames[] = {
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

      "arc_length_front_offset",
      "arc_length_back_offset",
      "from_arc_length_total",
      "to_arc_length_total",
      "arc_length_gap",
    };

    /// Vehicle class to transport the variable names
    struct FitlessAxialSegmentPairVarNames : public VarNames<CDCAxialSegmentPair> {

      /// Number of variables to be generated
      static const size_t nVars = size(fitlessAxialSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitlessAxialSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitlessAxialSegmentPairVarSet : public VarSet<FitlessAxialSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCAxialSegmentPair* ptrAxialSegmentPair) override;
    };
  }
}
