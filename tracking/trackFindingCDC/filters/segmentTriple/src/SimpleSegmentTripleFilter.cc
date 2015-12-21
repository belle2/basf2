/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void SimpleSegmentTripleFilter::beginEvent()
{
  m_simpleAxialSegmentPairFilter.beginEvent();
  Super::beginEvent();
}

void SimpleSegmentTripleFilter::initialize()
{
  m_simpleAxialSegmentPairFilter.initialize();
  Super::initialize();
}

void SimpleSegmentTripleFilter::terminate()
{
  Super::terminate();
  m_simpleAxialSegmentPairFilter.terminate();
}

Weight SimpleSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  assert(ptrStartSegment);
  assert(ptrMiddleSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCStereoRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //check if the middle segment lies within the acceptable bounds in angular deviation
  {
    //get the remembered fits
    const CDCTrajectory2D& startFit = m_simpleAxialSegmentPairFilter.getFittedTrajectory2D(startSegment);
    const CDCTrajectory2D& endFit = m_simpleAxialSegmentPairFilter.getFittedTrajectory2D(endSegment);

    //use only the first and last hit for this check
    const CDCRecoHit2D& firstHit = middleSegment.front();
    const CDCRecoHit2D& lastHit = middleSegment.back();

    Vector3D firstRecoPos = firstHit.reconstruct3D(startFit);
    Vector3D lastRecoPos = lastHit.reconstruct3D(endFit);

    const double toleranceFraction = 0.0;

    const CDCWire& firstWire = firstHit.getWire();
    const CDCWire& lastWire = lastHit.getWire();

    const bool agrees = firstWire.isInCellZBounds(firstRecoPos) and lastWire.isInCellZBounds(lastRecoPos);
  }

  // make more complex judgement on fitness

  // Get the combined fit of start and end axial segment
  const CDCTrajectory2D& fit = m_simpleAxialSegmentPairFilter.getFittedTrajectory2D(segmentTriple);

  // Check if the middle segment is actually coaligned with the trajectory
  EForwardBackward fbInfo = fit.isForwardOrBackwardTo(middleSegment);
  if (fbInfo != EForwardBackward::c_Forward) return NAN;

  //Reconstruct the
  CDCRecoSegment3D reconstructedMiddle;
  for (CDCStereoRecoSegment2D::const_iterator itRecoHits = middleSegment.begin();
       itRecoHits != middleSegment.end(); ++itRecoHits) {
    reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(*itRecoHits, fit));
    if (not reconstructedMiddle.back().isInCellZBounds()) {
      B2DEBUG(100, "  RecoHit out of CDC");
      //double d;
      //cin >> d;
      return NAN;
    }
  }

  // Fit the sz slope and intercept
  /*const CDCTrajectorySZ& trajectorySZ = */ getFittedTrajectorySZ(segmentTriple);

  Weight result = startSegment.size() + middleSegment.size() + endSegment.size();

  if (not std::isnan(result)) {

    m_simpleAxialSegmentPairFilter.getFittedTrajectory2D(segmentTriple);
    getFittedTrajectorySZ(segmentTriple);

  }

  return result;

}



const CDCTrajectorySZ& SimpleSegmentTripleFilter::getFittedTrajectorySZ(const CDCSegmentTriple& segmentTriple) const
{

  CDCTrajectorySZ& trajectorySZ = segmentTriple.getTrajectorySZ();

  if (not trajectorySZ.isFitted()) {

    const CDCTrajectory2D& trajectory2D = m_simpleAxialSegmentPairFilter.getFittedTrajectory2D(segmentTriple);

    const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
    const CDCStereoRecoSegment2D middleSegment = *ptrMiddleSegment;

    getSZFitter().update(trajectorySZ, middleSegment, trajectory2D);

  }

  return trajectorySZ;

}
