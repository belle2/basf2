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
    class CDCWireHitCluster;

    /// Names of the variables to be generated
    static constexpr char const* const basicClusterNames[] = {
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
      "variance_adc_count",
    };

    /// Vehicle class to transport the variable names
    struct BasicClusterVarNames : public VarNames<CDCWireHitCluster> {

      /// Number of variables to be generated
      static const size_t nVars = size(basicClusterNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicClusterNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a wire hit cluster
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicClusterVarSet : public VarSet<BasicClusterVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCWireHitCluster* ptrCluster) final;
    };
  }
}
