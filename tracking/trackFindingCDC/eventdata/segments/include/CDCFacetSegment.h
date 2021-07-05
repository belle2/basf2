/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRLWireHitSegment;

    /// A segment consisting of adjacent facets
    class  CDCFacetSegment : public  CDCSegment<CDCFacet> {

    public:
      /// Construct a train of facets from the given oriented wire hits
      static CDCFacetSegment create(const CDCRLWireHitSegment& rlWireHitSegment);
    };
  }
}
