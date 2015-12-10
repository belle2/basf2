/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_triple_relation/MCSegmentTripleRelationFilter.h>

using namespace std;
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

NeighborWeight MCSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& triple,
                                                         const CDCSegmentTriple& neighborTriple)
{
  CellWeight mcTripleWeight = m_mcSegmentTripleFilter(triple);
  CellWeight mcNeighborTripleWeight = m_mcSegmentTripleFilter(neighborTriple);

  bool mcDecision = (not isNotACell(mcTripleWeight)) and (not isNotACell(mcNeighborTripleWeight));
  return mcDecision ? - neighborTriple.getStart()->size() : NOT_A_NEIGHBOR;
}
