/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/MCSegmentTripleRelationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleRelationFilter::MCSegmentTripleRelationFilter(bool allowReverse) :
  m_mcSegmentTripleFilter(allowReverse)
{
}

void MCSegmentTripleRelationFilter::beginEvent()
{
  m_mcSegmentTripleFilter.beginEvent();
  Super::beginEvent();
}

void MCSegmentTripleRelationFilter::initialize()
{
  Super::initialize();
  m_mcSegmentTripleFilter.initialize();
}

void MCSegmentTripleRelationFilter::terminate()
{
  m_mcSegmentTripleFilter.terminate();
  Super::terminate();
}

Weight MCSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& triple,
                                                 const CDCSegmentTriple& neighborTriple)
{
  Weight mcTripleWeight = m_mcSegmentTripleFilter(triple);
  Weight mcNeighborTripleWeight = m_mcSegmentTripleFilter(neighborTriple);

  bool mcDecision = (not std::isnan(mcTripleWeight)) and (not std::isnan(mcNeighborTripleWeight));
  return mcDecision ? - neighborTriple.getStartSegment()->size() : NAN;
}
