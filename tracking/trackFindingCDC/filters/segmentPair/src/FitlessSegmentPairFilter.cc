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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterOnVarSet<SkimmedHitGapSegmentPairVarSet>;

Weight FitlessSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCSegment2D* ptrStartSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrEndSegment = segmentPair.getToSegment();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCSegment2D& startSegment = *ptrStartSegment;
  const CDCSegment2D& endSegment = *ptrEndSegment;

  double weight = startSegment.size() + endSegment.size();
  return Super::operator()(segmentPair) * weight;
}
