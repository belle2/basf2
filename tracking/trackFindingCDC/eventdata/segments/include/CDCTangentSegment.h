/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
