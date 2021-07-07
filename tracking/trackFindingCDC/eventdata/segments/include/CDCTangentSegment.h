/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// A segment consisting of adjacent tangents.
    class  CDCTangentSegment : public CDCSegment<CDCTangent> {

    public:
      /// Takes all distinct tangents from the facets in the path - Note! there is no particular order of the tangents in the segment.
      static CDCTangentSegment condense(const std::vector<const CDCFacet* >& facetPath);
    };
  }
}
