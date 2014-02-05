/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/TangentSegmentCreator.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

TangentSegmentCreator::TangentSegmentCreator() {;}

TangentSegmentCreator::~TangentSegmentCreator() {;}

void
TangentSegmentCreator::create(
  const std::vector<FacetSegment>& facetSegments,
  std::vector<TangentSegment>& tangentSegments
) const
{

  tangentSegments.reserve(tangentSegments.size() + facetSegments.size());

  BOOST_FOREACH(const FacetSegment & facetSegment, facetSegments) {

    //create a new recoSegment2D
    tangentSegments.push_back(TangentSegment());
    TangentSegment& tangentSegment = tangentSegments.back();

    create(facetSegment, tangentSegment);

    tangentSegment.ensureUnique();

  } // next facetsegment

}


void
TangentSegmentCreator::create(
  const FacetSegment& facetSegment,
  TangentSegment& tangentSegment
) const
{

  //tangentSegment.reserve(facetSegment.size() * 2 + 2);

  BOOST_FOREACH(const CDCRecoFacet * facet, facetSegment) {

    //the alignement of the tangents does not play a major role here
    tangentSegment.insert(tangentSegment.end(), facet->getStartToMiddle());
    tangentSegment.insert(tangentSegment.end(), facet->getStartToEnd());
    tangentSegment.insert(tangentSegment.end(), facet->getMiddleToEnd());

  }
}

