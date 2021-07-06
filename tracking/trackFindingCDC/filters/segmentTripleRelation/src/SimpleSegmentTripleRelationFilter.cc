/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& fromSegmentTriple
                                                     __attribute__((unused)),
                                                     const CDCSegmentTriple& toSegmentTriple)
{
  // Just let all found neighors pass since we have the same start -> end segment
  // and let the cellular automaton figure auto which is longest

  // can of course be adjusted by comparing the z components between
  // triple and neighbor triples

  // neighbor weight is a penalty for the overlap of the segments since we would
  // count it to times
  // could also be a better measure of fit quality
  return  -toSegmentTriple.getStartSegment()->size();
}
