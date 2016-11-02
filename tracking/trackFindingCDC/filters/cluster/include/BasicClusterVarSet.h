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

    /// Forward declaration of the CDCWireHitCluster.
    class CDCWireHitCluster;

    /// Names of the variables to be generated.
    constexpr static char const* const basicClusterNames[] = {
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

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class BasicClusterVarNames : public VarNames<CDCWireHitCluster> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicClusterNames);

      constexpr static char const* getName(int iName)
      {
        return basicClusterNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class BasicClusterVarSet : public VarSet<BasicClusterVarNames> {

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCWireHitCluster* cluster) override final;
    };
  }
}
