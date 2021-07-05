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

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseSegmentTripleRelationFilter>;

MCSegmentTripleRelationFilter::MCSegmentTripleRelationFilter(bool allowReverse) :
  m_mcSegmentTripleFilter(allowReverse)
{
  this->addProcessingSignalListener(&m_mcSegmentTripleFilter);
}

void MCSegmentTripleRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                     const std::string& prefix)
{
  m_mcSegmentTripleFilter.exposeParameters(moduleParamList, prefix);
}

void MCSegmentTripleRelationFilter::initialize()
{
  Super::initialize();
  setAllowReverse(m_mcSegmentTripleFilter.getAllowReverse());
}

Weight MCSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& fromTriple,
                                                 const CDCSegmentTriple& toTriple)
{
  if (fromTriple.getEndSegment() != toTriple.getStartSegment()) return NAN;

  Weight mcFromTripleWeight = m_mcSegmentTripleFilter(fromTriple);
  Weight mcToTripleWeight = m_mcSegmentTripleFilter(toTriple);

  ESign fromFBInfo = sign(mcFromTripleWeight);
  ESign toFBInfo = sign(mcToTripleWeight);

  if (not isValid(fromFBInfo) or not isValid(toFBInfo) or fromFBInfo != toFBInfo) return NAN;

  ESign commonFBInfo = fromFBInfo;

  Weight overlapWeight = toTriple.getStartSegment()->size();
  return commonFBInfo > 0 ? overlapWeight  : -overlapWeight;
}
