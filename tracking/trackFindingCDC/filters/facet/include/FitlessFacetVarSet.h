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
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>


#include <vector>
#include <string>
#include <cassert>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetFitlessNames[] = {
      "superlayer_id",
      // "oclock_delta",
      "abs_oclock_delta",
      "cell_extend",
      // "short_arm_is_crossing",
      // "long_arm_is_crossing",
      // "stable_twist",
      // "abs_layer_id_difference",
      "layer_id_difference",
      "crossing_id",
      "shape_id",
      // "is_forward_progression",
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

    public:
      /// Generate and assign the variables from the cluster
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
