/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

NeighborWeight
SimpleSegmentTripleRelationFilter::operator()(const CDCSegmentTriple&,
                                              const CDCSegmentTriple& neighborTriple)
{
  // Just let all found neighors pass since we have the same start -> end segment
  // and let the cellular automaton figure auto which is longest

  // can of course be adjusted by comparing the z components between
  // triple and neighbor triples

  // neighbor weight is a penalty for the overlap of the segments since we would
  // count it to times
  // could also be a better measure of fit quality
  return  -neighborTriple.getStart()->size();
}

