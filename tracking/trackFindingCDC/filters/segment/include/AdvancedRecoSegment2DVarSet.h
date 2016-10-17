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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>


#include <vector>
#include <string>
#include <cassert>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;


    /// Names of the variables to be generated.
    constexpr
    static char const* const advancedCDCRecoSegment2DVarNames[] = {
      "is_stereo",
      "superlayer_id",
      "size",

      "number_of_hit_layers",
      //"form_function",

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
      "fitted_d0"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class AdvancedCDCRecoSegment2DVarNames : public VarNames<CDCRecoSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 18;

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return advancedCDCRecoSegment2DVarNames[iName];
      }
    };

    /** Class that computes floating point variables from the segments.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class AdvancedCDCRecoSegment2DVarSet : public VarSet<AdvancedCDCRecoSegment2DVarNames> {

    public:
      /// Construct the peeler.
      explicit AdvancedCDCRecoSegment2DVarSet() : VarSet<AdvancedCDCRecoSegment2DVarNames>() { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const CDCRecoSegment2D* segment) override final;

      /// Initialize the peeler before event processing
      virtual void initialize() override final
      {
        prepareSuperLayerCenterArray();
      }
    };
  }
}
