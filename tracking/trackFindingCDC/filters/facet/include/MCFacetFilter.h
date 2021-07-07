/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;
    class CDCRLWireHitTriple;

    /// Filter for the constuction of good facets based on monte carlo information
    class MCFacetFilter : public MCSymmetric<BaseFacetFilter> {
    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseFacetFilter>;

    public:
      /**
       *  Constructor also setting the switch,
       *  if the reversed version of a facet (in comparision to MC truth) shall be accepted.
       */
      explicit MCFacetFilter(bool allowReverse = true);

    public:
      /**
       *  Main filter method returning the weight of the facet.
       *  Returns NAN, if the cell shall be rejected.
       */
      Weight operator()(const CDCFacet& facet) final;

    private:
      /// Indicated if the oriented triple is a correct hypotheses
      bool operator()(const CDCRLWireHitTriple& rlWireHitTriple, int maxInTrackHitIdDifference);
    };
  }
}
