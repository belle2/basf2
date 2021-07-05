/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Class filtering the neighborhood of facets with monte carlo information
    class MCFacetRelationFilter : public MCSymmetric<BaseFacetRelationFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseFacetRelationFilter>;

    public:
      /**
       *  Constructor also setting the switch if the reversed version of a facet
       *  (in comparision to MC truth) shall be accepted.
       */
      explicit MCFacetRelationFilter(bool allowReverse = false);

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the before event processing.
      void initialize() final;

    public:
      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return NAN if relation shall be rejected.
       */
      Weight operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet) final;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcFacetFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Monte Carlo cell filter to reject neighborhoods have false cells
      MCFacetFilter m_mcFacetFilter;
    };
  }
}
