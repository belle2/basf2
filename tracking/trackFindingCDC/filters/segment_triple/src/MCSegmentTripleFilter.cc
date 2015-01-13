/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCSegmentTripleFilter.h"

#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

MCSegmentTripleFilter::MCSegmentTripleFilter()
{
}



MCSegmentTripleFilter::~MCSegmentTripleFilter()
{
}



void MCSegmentTripleFilter::clear()
{
  m_mcAxialAxialSegmentPairFilter.clear();
}



void MCSegmentTripleFilter::initialize()
{
  m_mcAxialAxialSegmentPairFilter.initialize();
}



void MCSegmentTripleFilter::terminate()
{
  m_mcAxialAxialSegmentPairFilter.terminate();
}



CellWeight MCSegmentTripleFilter::isGoodSegmentTriple(const CDCSegmentTriple& segmentTriple, bool allowBackward) const
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
  CellWeight pairWeight =  m_mcAxialAxialSegmentPairFilter.isGoodAxialAxialSegmentPair(segmentTriple, allowBackward);
  if (isNotACell(pairWeight)) return NOT_A_CELL;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == INVALID_INFO) return NOT_A_CELL;

  ForwardBackwardInfo middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == INVALID_INFO) return NOT_A_CELL;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NOT_A_CELL;


  if ((startToMiddleFBInfo == FORWARD and middleToEndFBInfo == FORWARD) or
      (allowBackward and startToMiddleFBInfo == BACKWARD and middleToEndFBInfo == BACKWARD)) {

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


