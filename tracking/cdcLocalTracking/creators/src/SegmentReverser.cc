/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/SegmentReverser.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


SegmentReverser::SegmentReverser()
{
}

SegmentReverser::~SegmentReverser()
{
}


void
SegmentReverser::appendReversed(
  std::vector<CDCRecoSegment2D>& segments
) const
{

  size_t nSegments = segments.size();
  segments.reserve(2 * nSegments);

  for (size_t iSegment = 0; iSegment < nSegments; ++iSegment) {

    const CDCRecoSegment2D& segment = segments[iSegment];

    segments.push_back(CDCRecoSegment2D());
    CDCRecoSegment2D& newSegment = segments.back();

    reverse(segment, newSegment);
  }

}



inline void
SegmentReverser::reverse(
  const CDCRecoSegment2D& segment,
  CDCRecoSegment2D& reverseSegment
) const
{

  reverseSegment.reserve(segment.size());
  BOOST_REVERSE_FOREACH(const CDCRecoHit2D & recohit, segment) {

    reverseSegment.push_back(recohit.reversed());

  }
}











