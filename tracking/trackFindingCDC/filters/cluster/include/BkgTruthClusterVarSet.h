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
    constexpr
    static char const* const bkgTruthClusterNames[] = {
      "n_background_hits_truth",
      "background_fraction_truth",
      "weight",
      "truth",
    };

    /// Vehicle class to transport the variable names
    struct BkgTruthClusterVarNames : public VarNames<CDCWireHitCluster> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(bkgTruthClusterNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return bkgTruthClusterNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a wire hit cluster
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BkgTruthClusterVarSet : public VarSet<BkgTruthClusterVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BkgTruthClusterVarNames>;

    public:
      /// Signal the initialisation of the event processing - requires the Monte Carlo information
      void initialize() override;

      /// Signal the beginning of a new event - loads the Monte Carlo information
      void beginEvent() override;

      /// Generate and assign the contained variables
      bool extract(const CDCWireHitCluster* ptrCluster) final;
    };
  }
}
