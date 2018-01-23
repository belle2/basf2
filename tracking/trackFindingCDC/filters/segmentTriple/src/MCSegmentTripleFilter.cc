/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTriple/MCSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseSegmentTripleFilter>;

MCSegmentTripleFilter::MCSegmentTripleFilter(bool allowReverse) :
  Super(allowReverse),
  m_mcAxialSegmentPairFilter(allowReverse)
{
  this->addProcessingSignalListener(&m_mcAxialSegmentPairFilter);
}

void MCSegmentTripleFilter::exposeParameters(ModuleParamList* moduleParamList,
                                             const std::string& prefix)
{
  m_mcAxialSegmentPairFilter.exposeParameters(moduleParamList, prefix);
}

void MCSegmentTripleFilter::initialize()
{
  Super::initialize();
  setAllowReverse(m_mcAxialSegmentPairFilter.getAllowReverse());
}

Weight MCSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCStereoSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCAxialSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  const CDCAxialSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialSegment2D& endSegment = *ptrEndSegment;

  /// Recheck the axial axial compatability
  Weight pairWeight =
    m_mcAxialSegmentPairFilter(CDCAxialSegmentPair(ptrStartSegment, ptrEndSegment));

  if (std::isnan(pairWeight)) return NAN;

  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == EForwardBackward::c_Invalid) return NAN;

  EForwardBackward middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == EForwardBackward::c_Invalid) return NAN;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NAN;


  if ((startToMiddleFBInfo == EForwardBackward::c_Forward and middleToEndFBInfo == EForwardBackward::c_Forward) or
      (getAllowReverse() and startToMiddleFBInfo == EForwardBackward::c_Backward and middleToEndFBInfo == EForwardBackward::c_Backward)) {

    // Do fits
    setTrajectoryOf(segmentTriple);

    Weight cellWeight = startSegment.size() + middleSegment.size() + endSegment.size();
    return startToMiddleFBInfo > 0 ? cellWeight : -cellWeight;
  }

  return NAN;
}



void MCSegmentTripleFilter::setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const
{
  if (segmentTriple.getTrajectorySZ().isFitted()) {
    // SZ trajectory has been fitted before. Skipping
    // A fit sz trajectory implies a 2d trajectory to be fitted, but not the other way around
    return;
  }

  const CDCAxialSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  if (not ptrStartSegment) {
    B2WARNING("Start segment of segmentTriple is nullptr. Could not set fits.");
    return;
  }

  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();

  CDCTrajectory3D trajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);
  segmentTriple.setTrajectory3D(trajectory3D);
}
