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
    class CDCSegment2D;

    /// Names of the variables to be generated
    constexpr
    static char const* const advancedSegmentVarNames[] = {
      "is_stereo",
      "superlayer_id",
      "size",

      "number_of_hit_layers",

      "total_number_of_neighbors",
      "mean_number_of_neighbors",

      "total_drift_length",
      "mean_drift_length",
      "variance_drift_length",

      "total_inner_distance",
      "mean_inner_distance",
      "distance_to_superlayer_center",

      "total_adc_count",
      "mean_adc_count",
      "variance_adc_count",

      "number_of_taken_hits",

      "fit_prob",
      "fitted_d0",
    };

    /// Vehicle class to transport the variable names
    struct AdvancedSegmentVarNames : public VarNames<CDCSegment2D> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(advancedSegmentVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return advancedSegmentVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class AdvancedSegmentVarSet : public VarSet<AdvancedSegmentVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCSegment2D* segment) final;
    };
  }
}
