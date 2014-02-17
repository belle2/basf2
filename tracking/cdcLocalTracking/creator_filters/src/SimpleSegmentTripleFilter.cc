/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SimpleSegmentTripleFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

SimpleSegmentTripleFilter::SimpleSegmentTripleFilter() : SimpleAxialAxialSegmentPairFilter(), m_szFitter()
{

}

SimpleSegmentTripleFilter::~SimpleSegmentTripleFilter()
{
}


void SimpleSegmentTripleFilter::clear()
{
  SimpleAxialAxialSegmentPairFilter::clear();

}



CellWeight SimpleSegmentTripleFilter::isGoodSegmentTriple(const CDCSegmentTriple& segmentTriple) const
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


  //check if the middle segment lies within the acceptable bounds in angular deviation
  {
    //get the remembered fits
    const CDCTrajectory2D& startFit = SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(startSegment);
    const CDCTrajectory2D& endFit = SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(endSegment);

    //use only the first and last hit for this check
    const CDCRecoHit2D& firstHit = middleSegment.front();
    const CDCRecoHit2D& lastHit = middleSegment.back();

    Vector2D firstPosition = firstHit.getRecoPos2D();
    Vector2D lastPosition  = lastHit.getRecoPos2D();

    Vector2D firstReconstructed = startFit.getCloseSamePolarR(firstPosition);
    Vector2D lastReconstructed = endFit.getCloseSamePolarR(lastPosition);

    FloatType first_refToReconstructedAngle = firstPosition.angleWith(firstReconstructed);
    FloatType last_refToReconstructedAngle = lastPosition.angleWith(lastReconstructed);

    const CDCWire& firstWire = firstHit.getWire();
    const CDCWire& lastWire  = lastHit.getWire();

    //Both wires are stereo wires by construction
    //So they have both non zero twist
    //check if the first and last are within the acceptable twist bounds
    const std::pair<FloatType, FloatType>& first_phiRangeToRef = firstWire.getPhiRangeToRef();
    const std::pair<FloatType, FloatType>& last_phiRangeToRef  = lastWire.getPhiRangeToRef();

    const FloatType tolerance = 0.0;

    const bool agrees = (first_phiRangeToRef.first - tolerance) < first_refToReconstructedAngle and
                        first_refToReconstructedAngle < (first_phiRangeToRef.second + tolerance) and

                        (last_phiRangeToRef.first - tolerance) < last_refToReconstructedAngle and
                        last_refToReconstructedAngle < (last_phiRangeToRef.second + tolerance) ;

    //B2DEBUG(100,"    first_rangeToRefAngle.first < first_refToReconstructedAngle  = "
    //              << (first_rangeToRefAngle.first < first_refToReconstructedAngle) );

    //B2DEBUG(100,"    first_refToReconstructedAngle < first_rangeToRefAngle.second = "
    //              << (first_refToReconstructedAngle < first_rangeToRefAngle.second) );

    //B2DEBUG(100,"    last_rangeToRefAngle.first < last_refToReconstructedAngle    = "
    //              << (last_rangeToRefAngle.first < last_refToReconstructedAngle) );

    //B2DEBUG(100,"    last_refToReconstructedAngle < last_rangeToRefAngle.second   = "
    //              << (last_refToReconstructedAngle < last_rangeToRefAngle.second) );


    if (not agrees) {
      B2DEBUG(100, "    first_rangeToRefAngle.first   = " <<  first_phiRangeToRef.first);
      B2DEBUG(100, "    first_refToReconstructedAngle = " <<  first_refToReconstructedAngle);
      B2DEBUG(100, "    first_rangeToRefAngle.second  = " <<  first_phiRangeToRef.second);

      B2DEBUG(100, "    firstPosition                 = " <<  firstPosition);
      B2DEBUG(100, "    firstReconstructed            = " <<  firstReconstructed);

      B2DEBUG(100, "    last_rangeToRefAngle.first    = " <<  last_phiRangeToRef.first);
      B2DEBUG(100, "    last_refToReconstructedAngle  = " <<  last_refToReconstructedAngle);
      B2DEBUG(100, "    last_rangeToRefAngle.second   = " <<  last_phiRangeToRef.second);

      B2DEBUG(100, "    lastPosition                  = " <<  firstPosition);
      B2DEBUG(100, "    lastReconstructed             = " <<  lastReconstructed);

      B2DEBUG(100, "  MiddleSegment rejected because of angular mismatch");


      //double d;
      //cin >> d;
      return NOT_A_CELL;
    }
    //now check if the the fit and the middle segment are coaligned - is here the right point to check for this ??
    //if ( not middleSegment.isForwardFit(startFit) ) return false;

  }

  // make more complex judgement on fitness

  // Get the combined fit of start and end axial segment
  const CDCTrajectory2D& fit = SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(segmentTriple);

  // Check if the middle segment is actually coaligned with the trajectory
  if (not middleSegment.isForwardTrajectory(fit)) return NOT_A_CELL;

  //Reconstruct the
  CDCRecoSegment3D reconstructedMiddle;
  for (CDCStereoRecoSegment2D::const_iterator itRecoHits = middleSegment.begin();
       itRecoHits != middleSegment.end(); ++itRecoHits) {
    reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(*itRecoHits, fit));
    if (not reconstructedMiddle.back().isInCDC()) {
      B2DEBUG(100, "  RecoHit out of CDC");
      //double d;
      //cin >> d;
      return NOT_A_CELL;
    }
  }

  const CDCTrajectorySZ& trajectorySZ = getFittedTrajectorySZ(segmentTriple);

  FloatType squaredDistance = reconstructedMiddle.getSquaredZDist(trajectorySZ);

  {
    B2DEBUG(100, "  ZDistance of the middleSegment = " << squaredDistance);

  }

  CellState result = startSegment.size() + middleSegment.size() + endSegment.size();

  if (not isNotACell(result)) {

    SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(segmentTriple);
    getFittedTrajectorySZ(segmentTriple);

  }

  return result;

}



const CDCTrajectorySZ& SimpleSegmentTripleFilter::getFittedTrajectorySZ(const CDCSegmentTriple& segmentTriple) const
{

  CDCTrajectorySZ& trajectorySZ = segmentTriple.getTrajectorySZ();

  if (not trajectorySZ.isFitted()) {

    const CDCTrajectory2D& trajectory2D = getFittedTrajectory2D(segmentTriple);

    const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
    const CDCStereoRecoSegment2D middleSegment = *ptrMiddleSegment;

    //recostruct the stereo segment with the hit
    CDCRecoSegment3D reconstructedMiddle;
    for (CDCStereoRecoSegment2D::const_iterator itRecoHits = middleSegment.begin();
         itRecoHits != middleSegment.end(); ++itRecoHits) {
      reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(*itRecoHits, trajectory2D));
    }

    getSZFitter().update(trajectorySZ, reconstructedMiddle);

  }

  return trajectorySZ;

}



