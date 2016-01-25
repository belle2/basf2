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
#include <tracking/trackFindingCDC/filters/facet/FitlessFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>


#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetFitlessNames[] = {
      "superlayer_id",
      "shape",
      "abs_shape",
      "start_rlinfo",
      "start_drift_length",
      "start_drift_length_sigma",
      "middle_rlinfo",
      "middle_drift_length",
      "middle_drift_length_sigma",
      "end_rlinfo",
      "end_drift_length",
      "end_drift_length_sigma",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet
     */
    class FitlessFacetVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetFitlessNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetFitlessNames[iName];
      }
    };

    /** Class that computes floating point variables from a facet.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitlessFacetVarSet : public VarSet<FitlessFacetVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitlessFacetVarNames>;

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit FitlessFacetVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCFacet* facet) override final;

    };
  }
}
