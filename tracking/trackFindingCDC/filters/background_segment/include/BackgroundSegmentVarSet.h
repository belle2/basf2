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

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;


    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const backgroundSegmentNames[] = {
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
      "variance_adc_count"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class BackgroundSegmentVarNames : public VarNames<CDCRecoSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 15;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return backgroundSegmentNames[iName];
      }
    };

    /** Class that computes floating point variables from a pair of track and segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class BackgroundSegmentVarSet : public VarSet<BackgroundSegmentVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      BackgroundSegmentVarSet(const std::string& prefix = "") : VarSet<BackgroundSegmentVarNames>(prefix) { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const CDCRecoSegment2D* segment) IF_NOT_CINT(override final);

      /// Initialize the peeler before event processing
      virtual void initialize() IF_NOT_CINT(override final)
      {
        prepareSuperLayerCenterArray();
      }
    };
  }
}
