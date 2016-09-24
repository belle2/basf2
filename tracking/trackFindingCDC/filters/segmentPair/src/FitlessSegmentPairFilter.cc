/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairFilter.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;


Weight FitlessSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getToSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  double weight = startSegment.size() + endSegment.size();
  return Super::operator()(segmentPair) * weight;
}
