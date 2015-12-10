/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_pair_relation/MCSegmentPairRelationFilter.h>

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


NeighborWeight
MCSegmentPairRelationFilter::operator()(const CDCSegmentPair& fromSegmentPair,
                                        const CDCSegmentPair& toSegmentPair)
{
  CellWeight mcFromPairWeight = m_mcSegmentPairFilter(fromSegmentPair);
  CellWeight mcToPairWeight = m_mcSegmentPairFilter(toSegmentPair);

  bool mcDecision = (not isNotACell(mcFromPairWeight)) and (not isNotACell(mcToPairWeight));
  return mcDecision ? -toSegmentPair.getStartSegment()->size() : NOT_A_NEIGHBOR;
}
