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

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <array>
#include <vector>
#include <string>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Class filtering the neighborhood of facets based on chi2 from a fit both facets.
    class Chi2FacetRelationFilter : public BaseFacetRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetRelationFilter;

    public:
      /// Constructor with the default chi2 cut value and width parameter
      Chi2FacetRelationFilter();

      /// Constructor using given chi2 cut value and width parameter
      Chi2FacetRelationFilter(double chi2Cut, double penaltyWidth);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialise the parameter caches before the processing starts
      void initialize() final;

    public:
      /// Main filter method returning the weight of the neighborhood relation.
      Weight operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet) final;

    private: // Parameters
      /// Parameter : The chi2 cut values distinguished by superlayer
      std::vector<double> m_param_chi2CutByISuperLayer{130.0};  // first guess was 350;

      /// Parameter : The chi2 cut values distinguished by superlayer
      double m_param_penaltyFactor = 200.0 / 130.0; // first guess was 800 / 350;

    private: // Cached values
      /// Memory for the chi2 cut values distinguished by superlayer
      std::array<double, ISuperLayerUtil::c_N> m_chi2CutByISuperLayer{};

      /// Memory for the chi2 cut values distinguished by superlayer
      std::array<double, ISuperLayerUtil::c_N> m_penaltyWidthByISuperLayer{};
    };
  }
}
