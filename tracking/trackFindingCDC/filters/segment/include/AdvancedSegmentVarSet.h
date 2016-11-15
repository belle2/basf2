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
    class CDCSegment2D;


    /// Names of the variables to be generated.
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

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class AdvancedSegmentVarNames : public VarNames<CDCSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(advancedSegmentVarNames);

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return advancedSegmentVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from the segments.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class AdvancedSegmentVarSet : public VarSet<AdvancedSegmentVarNames> {

    public:
      /// Generate and assign the variables from the pair
      bool extract(const CDCSegment2D* segment) final;
    };
  }
}
