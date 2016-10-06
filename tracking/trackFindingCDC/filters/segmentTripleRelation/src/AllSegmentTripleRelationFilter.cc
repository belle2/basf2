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

using namespace Belle2;
using namespace TrackFindingCDC;


Weight
AllSegmentTripleRelationFilter::operator()(const CDCSegmentTriple&,
                                           const CDCSegmentTriple& toTriple)
{
  // Just let all found neighors pass for the base implementation
  // with the default weight
  return  -toTriple.getStartSegment()->size();
}
