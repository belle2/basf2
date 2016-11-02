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
    /// Filter for the constuction of good facets based on simple criterions.
    class RealisticFacetFilter : public Filter<CDCFacet> {

    private:
      /// Type of the super class
      using Super = Filter<CDCFacet>;

    public:
      /// Constructor using default direction of flight deviation cut off.
      RealisticFacetFilter();

      /// Constructor using given direction of flight deviation cut off.
      explicit RealisticFacetFilter(double phiPullCut);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override;

    public:
      /**
       *  Main filter method returning the weight of the facet
       *  Returns NAN if the cell shall be rejected.
       */
      virtual
      Weight operator()(const CDCFacet& facet) override final;

    private:
      /// Memory for the pull cu
      double m_param_phiPullCut;

    }; // end class RealisticFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
