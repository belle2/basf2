/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/AllSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& fromSegmentTriple
                                                  __attribute__((unused)),
                                                  const CDCSegmentTriple& toSegmentTriple)
{
  // Just let all found neighors pass for the base implementation
  // with the default weight
  return  -toSegmentTriple.getStartSegment()->size();
}
