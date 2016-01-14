/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MCSegmentPairRelationFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentPairRelationFilter::MCSegmentPairRelationFilter(bool allowReverse) :
  Super(allowReverse),
  m_mcSegmentPairFilter(allowReverse)
{
}


void MCSegmentPairRelationFilter::beginEvent()
{
  m_mcSegmentPairFilter.beginEvent();
  Super::beginEvent();
}



void MCSegmentPairRelationFilter::initialize()
{
  Super::initialize();
  m_mcSegmentPairFilter.initialize();
}



void MCSegmentPairRelationFilter::terminate()
{
  m_mcSegmentPairFilter.terminate();
  Super::terminate();
}


Weight
MCSegmentPairRelationFilter::operator()(const CDCSegmentPair& fromSegmentPair,
                                        const CDCSegmentPair& toSegmentPair)
{
  Weight mcFromPairWeight = m_mcSegmentPairFilter(fromSegmentPair);
  Weight mcToPairWeight = m_mcSegmentPairFilter(toSegmentPair);

  bool mcDecision = (not std::isnan(mcFromPairWeight)) and (not std::isnan(mcToPairWeight));
  return mcDecision ? -toSegmentPair.getFromSegment()->size() : NAN;
}
