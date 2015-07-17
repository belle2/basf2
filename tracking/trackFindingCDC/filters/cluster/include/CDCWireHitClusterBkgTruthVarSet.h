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
    static char const* const clusterBkgTruthNames[4] = {
      "n_background_hits_truth",
      "background_fraction_truth",
      "weight",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class CDCWireHitClusterBkgTruthVarNames : public VarNames<CDCWireHitCluster> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 4;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return clusterBkgTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCWireHitClusterBkgTruthVarSet : public VarSet<CDCWireHitClusterBkgTruthVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      explicit CDCWireHitClusterBkgTruthVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCWireHitCluster* cluster) IF_NOT_CINT(override final);

    };
  }
}
