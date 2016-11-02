/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of facets based on chi2 from a fit both facets.
    class Chi2FacetRelationFilter : public BaseFacetRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetRelationFilter;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Constructor with the default chi2 cut value and width parameter
      Chi2FacetRelationFilter() = default;

      /// Constructor using given chi2 cut value and width parameter
      explicit Chi2FacetRelationFilter(double chi2Cut, double penaltyWidth);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override;
    public:
      /// Main filter method returning the weight of the neighborhood relation.
      virtual Weight operator()(const CDCFacet& fromFacet,
                                const CDCFacet& toFacet) override final;

    private:
      /// Memory for the chi2 cut value
      double m_param_chi2Cut = 130; // first guess was 350;

      /// Memory for the width parameter to translate the chi2 value to a weight penatlity
      double m_param_penaltyWidth = 200; // first guess was 800;

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
