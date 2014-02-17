/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SimpleAxialAxialSegmentPairFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

SimpleAxialAxialSegmentPairFilter::SimpleAxialAxialSegmentPairFilter() : m_riemannFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

SimpleAxialAxialSegmentPairFilter::~SimpleAxialAxialSegmentPairFilter()
{

}


void SimpleAxialAxialSegmentPairFilter::clear()
{

}


CellWeight SimpleAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("SimpleAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return NOT_A_CELL;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("SimpleAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  //B2DEBUG(100,"    startSegment.front() = " <<  startSegment.front() );
  //B2DEBUG(100,"    startSegment.back()  = " <<  startSegment.back() );

  //B2DEBUG(100,"    endSegment.front()   = " <<  endSegment.front() );
  //B2DEBUG(100,"    endSegment.back()    = " <<  endSegment.back() );

  //B2DEBUG(100,"    startSegment.getStartPerpS(startFit) = " <<  startSegment.getStartPerpS(startFit) );
  //B2DEBUG(100,"    startSegment.getEndPerpS(startFit)   = " <<  startSegment.getEndPerpS(startFit) );

  //B2DEBUG(100,"    endSegment.getStartPerpS(startFit)   = " <<  endSegment.getStartPerpS(startFit) );
  //B2DEBUG(100,"    endSegment.getEndPerpS(startFit)     = " <<  endSegment.getEndPerpS(startFit) );

  //B2DEBUG(100,"    Check startSegment isForwardFit " << startSegment.isForwardFit(startFit));
  //B2DEBUG(100,"    Check endSegment isForwardFit " << endSegment.isForwardFit(startFit));

  //alignment cut - nonnegotiable
  if (not endSegment.isForwardTrajectory(startFit)) return NOT_A_CELL;

  //FloatType distanceAverage = ( endSegment.getStartPerpS(startFit) +
  //                              endSegment.getEndPerpS(startFit)    ) / 2;


  //check if end segment is in forward direction of startSegment
  //reference point was already set in the fit to the first hit of the startSegment
  //alignment cut - nonnegotiable
  if (endSegment.getEndPerpS(startFit)   < startSegment.getEndPerpS(startFit) or
      endSegment.getStartPerpS(startFit) < startSegment.getStartPerpS(startFit)) return NOT_A_CELL;


  //check if the last hit of the reco hit lies further in travel direction than the first
  //FloatType distanceLastToFirst = endSegment.getEndPerpS(startFit) -
  //                                endSegment.getStartPerpS(startFit) ;

  Vector2D startCOM = startSegment.getCenterOfMass2D();
  Vector2D endCOM   = endSegment.getCenterOfMass2D();

  //B2DEBUG(100,"  Check distanceAverage " << distanceAverage);
  //B2DEBUG(100,"  Check distanceLastToFirst " << distanceLastToFirst);

  //difference in position at the forward extrapolation point
  Vector2D endCenter = endFit.getClosest(endCOM);
  Vector2D pointOnFromTrack = startFit.getCloseSamePolarR(endCenter);

  //difference in flight direction
  Vector2D startDirection  = startFit.getUnitMom2D(pointOnFromTrack);
  Vector2D endDirection    = endFit.getUnitMom2D(endCenter);

  //difference in momentum ?
  //marked as usused, because they should not generate a warning
  //the variables are a reminder which variables can be used in the cut
  FloatType startMom __attribute__((unused)) = startFit.getAbsMom2D();
  FloatType endMom __attribute__((unused)) = endFit.getAbsMom2D();

  //check if end segment is in forward direction of startSegment


  //make a cut - make this more sophisticated at some point
  double cosDeviation = endCenter.cosWith(pointOnFromTrack);
  double tolerance = cos(PI / 180);

  return startSegment.size() + endSegment.size();

  if (cosDeviation > tolerance) {
    return startSegment.size() + endSegment.size();
  } else {
    return NOT_A_CELL;
  }

}



const CDCTrajectory2D& SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}



const CDCTrajectory2D& SimpleAxialAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const
{
  CDCTrajectory2D& trajectory2D = axialAxialSegmentPair.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, axialAxialSegmentPair);
  }
  return trajectory2D;
}



