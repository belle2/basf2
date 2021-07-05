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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseSegmentPairRelationFilter>;

MCSegmentPairRelationFilter::MCSegmentPairRelationFilter(bool allowReverse)
  : Super(allowReverse)
  , m_mcSegmentPairFilter(allowReverse)
{
  this->addProcessingSignalListener(&m_mcSegmentPairFilter);
}

void MCSegmentPairRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                   const std::string& prefix)
{
  m_mcSegmentPairFilter.exposeParameters(moduleParamList, prefix);
}

void MCSegmentPairRelationFilter::initialize()
{
  Super::initialize();
  setAllowReverse(m_mcSegmentPairFilter.getAllowReverse());
}

Weight MCSegmentPairRelationFilter::operator()(const CDCSegmentPair& fromSegmentPair,
                                               const CDCSegmentPair& toSegmentPair)
{
  Weight mcFromPairWeight = m_mcSegmentPairFilter(fromSegmentPair);
  Weight mcToPairWeight = m_mcSegmentPairFilter(toSegmentPair);

  ESign fromFBInfo = sign(mcFromPairWeight);
  ESign toFBInfo = sign(mcToPairWeight);

  if (isValid(fromFBInfo) and isValid(toFBInfo) and fromFBInfo == toFBInfo) {
    ESign commonFBInfo = fromFBInfo;

    size_t fromOverlapSize = fromSegmentPair.getToSegment()->size();
    size_t fromSize = fromOverlapSize + fromSegmentPair.getFromSegment()->size();
    Weight fromWeight = fromSegmentPair.getAutomatonCell().getCellWeight();

    size_t toOverlapSize = toSegmentPair.getFromSegment()->size();
    size_t toSize = toOverlapSize + toSegmentPair.getToSegment()->size();
    Weight toWeight = toSegmentPair.getAutomatonCell().getCellWeight();
    double overlapWeight =
      (fromWeight * fromOverlapSize / fromSize + toWeight * toOverlapSize / toSize) / 2.0;

    if (commonFBInfo < 0 and Super::getAllowReverse()) {
      return -std::fabs(overlapWeight);
    } else if (commonFBInfo > 0) {
      return std::fabs(overlapWeight);
    } else {
      return NAN;
    }
  } else {
    return NAN;
  }
}
