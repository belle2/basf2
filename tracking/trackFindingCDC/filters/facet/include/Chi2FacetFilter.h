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

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of good facets based in the chi2 fit
    class Chi2FacetFilter : public Filter<CDCFacet> {

    private:
      /// Type of the super class
      using Super = Filter<CDCFacet>;

    public:
      /// Constructor with the default chi2 cut value and width parameter
      Chi2FacetFilter();

      /// Constructor using given chi2 cut value and width parameter
      explicit Chi2FacetFilter(double chi2Cut, double penaltyWidth);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override;

    public:
      /**
       *  Main filter method returning the weight of the facet
       *  Returns NAN if the cell shall be rejected.
       */
      virtual Weight operator()(const CDCFacet& facet) override final;

    private:
      /// Memory for the chi2 cut value
      double m_param_chi2Cut = 75;

      /// Memory for the width parameter to translate the chi2 value to a weight penatlity
      double m_param_penaltyWidth = 120.0;

    }; // end class RealisticFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
