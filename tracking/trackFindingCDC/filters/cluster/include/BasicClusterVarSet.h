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
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
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
