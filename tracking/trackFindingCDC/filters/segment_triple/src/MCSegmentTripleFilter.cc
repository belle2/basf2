/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_triple/MCSegmentTripleFilter.h>

#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleFilter::MCSegmentTripleFilter(bool allowReverse) :
  Super(allowReverse),
  m_mcAxialSegmentPairFilter(allowReverse)
{
}


void MCSegmentTripleFilter::clear()
{
  m_mcAxialSegmentPairFilter.clear();
  Super::clear();
}



void MCSegmentTripleFilter::initialize()
{
  Super::initialize();
  m_mcAxialSegmentPairFilter.initialize();
}



void MCSegmentTripleFilter::terminate()
{
  m_mcAxialSegmentPairFilter.terminate();
  Super::terminate();
}



CellWeight MCSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as start segment");
    return NOT_A_CELL;
  }
  if (ptrMiddleSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as middle segment");
    return NOT_A_CELL;
  }
  if (ptrEndSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  /// Recheck the axial axial compatability
  CellWeight pairWeight =  m_mcAxialSegmentPairFilter(segmentTriple);
  if (isNotACell(pairWeight)) return NOT_A_CELL;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == EForwardBackward::c_Invalid) return NOT_A_CELL;

  EForwardBackward middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == EForwardBackward::c_Invalid) return NOT_A_CELL;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NOT_A_CELL;


  if ((startToMiddleFBInfo == EForwardBackward::c_Forward and middleToEndFBInfo == EForwardBackward::c_Forward) or
      (getAllowReverse() and startToMiddleFBInfo == EForwardBackward::c_Backward and middleToEndFBInfo == EForwardBackward::c_Backward)) {

    // Do fits
    setTrajectoryOf(segmentTriple);

    CellState cellWeight = startSegment.size() + middleSegment.size() + endSegment.size();
    return cellWeight;

  }

  return NOT_A_CELL;
}



void MCSegmentTripleFilter::setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const
{
  if (segmentTriple.getTrajectorySZ().isFitted()) {
    // SZ trajectory has been fitted before. Skipping
    // A fit sz trajectory implies a 2d trajectory to be fitted, but not the other way around
    return;
  }

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  if (not ptrStartSegment) {
    B2WARNING("Start segment of segmentTriple is nullptr. Could not set fits.");
    return;
  }

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  CDCTrajectory3D trajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);

  segmentTriple.setTrajectory2D(trajectory3D.getTrajectory2D());
  segmentTriple.setTrajectorySZ(trajectory3D.getTrajectorySZ());

}
