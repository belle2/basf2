/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
