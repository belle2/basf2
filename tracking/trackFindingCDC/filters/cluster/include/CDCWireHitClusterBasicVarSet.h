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
    /// Forward declaration of the CDCWireHitCluster.
    class CDCWireHitCluster;


    /// Names of the variables to be generated.
    constexpr
    static char const* const clusterBasicNames[14] = {
      "is_stereo",
      "superlayer_id",
      "size",

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
    class CDCWireHitClusterBasicVarNames : public VarNames<CDCWireHitCluster> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 14;

      /// Names of the variables to be generated.
      // constexpr
      // static char const* const clusterNames[11] = {
      //  "avg_n_neignbors", // TODO: correct name and ordering once we can retrain the analysis.
      //  "distance_to_superlayer_center",
      //  "is_stereo",
      //  "mean_drift_length",
      //  "mean_inner_distance",
      //  "size",
      //  "superlayer_id",
      //  "total_drift_length",
      //  "total_inner_distance",
      //  "total_n_neighbors",
      //  "variance_drift_length",
      // };

      constexpr
      static char const* getName(int iName)
      {
        return clusterBasicNames[iName];
      }
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCWireHitClusterBasicVarSet : public VarSet<CDCWireHitClusterBasicVarNames> {

    public:
      /// Construct the peeler.
      explicit CDCWireHitClusterBasicVarSet();

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCWireHitCluster* cluster) override final;

      /// Initialize the peeler before event processing
      virtual void initialize() override final
      {
        prepareSuperLayerCenterArray();
      }
    };
  }
}
