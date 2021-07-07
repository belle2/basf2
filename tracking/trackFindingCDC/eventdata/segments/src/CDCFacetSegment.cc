/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/logging/Logger.h>

#include <iterator>
#include <vector>
#include <stddef.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }
}

using namespace Belle2;
using namespace TrackFindingCDC;

CDCFacetSegment CDCFacetSegment::create(const CDCRLWireHitSegment& rlWireHitSegment)
{
  CDCFacetSegment facetSegment;
  size_t nRLWireHits = rlWireHitSegment.size();
  if (nRLWireHits < 3) return facetSegment;

  facetSegment.reserve(nRLWireHits - 2);

  // Make tangents from pairs of hits along the segment.
  transform_adjacent_triples(rlWireHitSegment.begin(),
                             rlWireHitSegment.end(),
                             back_inserter(facetSegment),
                             [](const CDCRLWireHit & firstRLWireHit,
                                const CDCRLWireHit & secondRLWireHit,
  const CDCRLWireHit & thirdRLWireHit) {
    return CDCFacet(firstRLWireHit, secondRLWireHit, thirdRLWireHit);
  });

  if (facetSegment.size() + 2 != rlWireHitSegment.size()) {
    B2ERROR("Wrong number of facets created.");
  }

  facetSegment.setAliasScore(rlWireHitSegment.getAliasScore());
  facetSegment.setTrajectory2D(rlWireHitSegment.getTrajectory2D());
  return facetSegment;
}
