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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Filter for the constuction of good facets based in the chi2 fit
    class Chi2FacetFilter : public BaseFacetFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetFilter;

    public:
      /// Constructor with the default chi2 cut value and width parameter
      Chi2FacetFilter();

      /// Constructor using given chi2 cut value and width parameter
      Chi2FacetFilter(double chi2Cut, double penaltyWidth);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialise the parameter caches before the processing starts
      void initialize() final;

    public:
      /**
       *  Main filter method returning the weight of the facet
       *  Returns NAN if the cell shall be rejected.
       */
      Weight operator()(const CDCFacet& facet) final;

    private: // Parameters
      /// Parameter : The chi2 cut values distinguished by superlayer
      std::vector<double> m_param_chi2CutByISuperLayer{75.0};

      /// Parameter : The chi2 cut values distinguished by superlayer
      double m_param_penaltyFactor = 120.0 / 75.0;

    private: // Cached values
      /// Memory for the chi2 cut values distinguished by superlayer
      std::array<double, ISuperLayerUtil::c_N> m_chi2CutByISuperLayer{};

      /// Memory for the chi2 cut values distinguished by superlayer
      std::array<double, ISuperLayerUtil::c_N> m_penaltyWidthByISuperLayer{};
    };
  }
}
