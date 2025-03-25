/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCFacet.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCTangent.h>

using namespace Belle2;
using namespace TrackingUtilities;

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
      //the alignment of the tangents does not play a major role here
    }
    tangentSegment.push_back(facetPath.back()->getMiddleToEnd());
  }
  return tangentSegment;

}
