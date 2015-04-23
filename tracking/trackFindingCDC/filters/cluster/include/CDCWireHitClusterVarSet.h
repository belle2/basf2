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
    /// Forward declaration of the CDCWireHitCluster.
    class CDCWireHitCluster;


    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const clusterNames[11] = {
      "avg_n_neignbors", // TODO: correct name and ordering once we can retrain the analysis.
      "distance_to_superlayer_center",
      "is_stereo",
      "mean_drift_length",
      "mean_inner_distance",
      "size",
      "superlayer_id",
      "total_drift_length",
      "total_inner_distance",
      "total_n_neighbors",
      "variance_drift_length",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class CDCWireHitClusterVarNames : public VarNames<CDCWireHitCluster> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 11;

      /// Names of the variables to be generated.
      // IF_NOT_CINT(constexpr)
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

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return clusterNames[iName];
      }
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCWireHitClusterVarSet : public VarSet<CDCWireHitClusterVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      CDCWireHitClusterVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCWireHitCluster* cluster) IF_NOT_CINT(override final);

      /// Initialize the peeler before event processing
      virtual void initialize() IF_NOT_CINT(override final);

    private:
      /// Prepare the superlayer center array with information coming from the CDCWireTopology.
      void prepareSuperLayerCenterArray();

    private:
      /// The cylinder radius of the super layer centers
      std::vector<double> m_superLayerCenters;
    };
  }
}
