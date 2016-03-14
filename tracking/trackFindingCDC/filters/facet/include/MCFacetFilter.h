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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of good facets based on monte carlo information
    class MCFacetFilter : public MCSymmetricFilterMixin<Filter<CDCFacet> > {
    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<CDCFacet> > Super;

    public:
      /** Constructor also setting the switch ,
       *  if the reversed version of a facet (in comparision to MC truth) shall be accepted.
       */
      explicit MCFacetFilter(bool allowReverse = true) : Super(allowReverse) {;}

    public:
      /** Main filter method returning the weight of the facet.
       *  Returns NOT_A_CELL if the cell shall be rejected.
       */
      virtual CellWeight operator()(const CDCFacet& facet) IF_NOT_CINT(override final);

    private:
      /// Indicated if the oriented triple is a correct hypotheses
      bool operator()(const CDCRLWireHitTriple& rlWireHit, int inTrackHitDistanceTolerance = 99999);

    }; // end class MCFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
