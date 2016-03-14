/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of facets with monte carlo information
    class MCFacetRelationFilter : public MCSymmetricFilterMixin<Filter<Relation<CDCFacet> > > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<Relation<CDCFacet> > > Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor also setting the switch if the reversed version of a facet
       *  (in comparision to MC truth) shall be accepted.
       */
      MCFacetRelationFilter(bool allowReverse = true) :
        Super(allowReverse), m_mcFacetFilter(allowReverse) {;}

    public:
      /// May be used to clear information from former events. Currently unused.
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void terminate() IF_NOT_CINT(override final);

    public:
      /** Main filter method returning the weight of the neighborhood relation.
       *  Return NOT_A_NEIGHBOR if relation shall be rejected.
       */
      virtual NeighborWeight operator()(const CDCFacet& fromFacet,
                                        const CDCFacet& toFacet) override final;

    public:
      /// Setter for the allow reverse parameter
      virtual void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcFacetFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Monte Carlo cell filter to reject neighborhoods have false cells
      MCFacetFilter m_mcFacetFilter;

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
