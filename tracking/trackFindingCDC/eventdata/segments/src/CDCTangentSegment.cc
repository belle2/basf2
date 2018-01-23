/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCTangentSegment CDCTangentSegment::condense(const std::vector<const CDCFacet* >& facetPath)
{
  CDCTangentSegment tangentSegment;

  tangentSegment.reserve(facetPath.size() + 1);
  if (not facetPath.empty()) {
    for (const CDCFacet* ptrFacet : facetPath) {
      if (not ptrFacet) continue;

      const CDCFacet& facet = *ptrFacet;

      tangentSegment.push_back(facet.getStartToMiddle());
      //tangentSegment.push_back(facet.getStartToEnd());
      //the alignement of the tangents does not play a major role here
    }
    tangentSegment.push_back(facetPath.back()->getMiddleToEnd());
  }
  return tangentSegment;

}
