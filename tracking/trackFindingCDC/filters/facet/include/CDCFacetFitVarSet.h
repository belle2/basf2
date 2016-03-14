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

#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>
#include <tracking/trackFindingCDC/filters/facet/CDCFacetFitlessVarSet.h>

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const facetFitNames[] = {
      "start_phi",
      "start_phi_sigma",
      "start_phi_pull",

      "middle_phi",
      "middle_phi_sigma",
      "middle_phi_pull",

      "end_phi",
      "end_phi_sigma",
      "end_phi_pull"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet
     */
    class CDCFacetFitVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetFitNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return facetFitNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef CDCFacetFitlessVarSet NestedVarSet;
    };

    /** Class that computes floating point variables from a facet.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class CDCFacetFitVarSet : public VarSet<CDCFacetFitVarNames> {

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit CDCFacetFitVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet
      virtual bool extract(const CDCFacet* facet) IF_NOT_CINT(override final);

    private:
      /// Fitless filter for the feasibility cut of the right left passage information.
      FitlessFacetFilter m_fitlessFacetFilter;
    };
  }
}
