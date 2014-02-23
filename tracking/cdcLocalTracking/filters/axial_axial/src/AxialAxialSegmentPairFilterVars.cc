/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AxialAxialSegmentPairFilterVars.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;



AxialAxialSegmentPairFilterVars::AxialAxialSegmentPairFilterVars()
{
}



AxialAxialSegmentPairFilterVars::~AxialAxialSegmentPairFilterVars()
{
}



bool AxialAxialSegmentPairFilterVars::fill(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return false;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return false;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCTrajectory2D& startFit =  startSegment.getTrajectory2D();
  const CDCTrajectory2D& endFit = endSegment.getTrajectory2D();

  m_startSegment_startPerpS_startFit = startSegment.getStartPerpS(startFit);
  m_startSegment_endPerpS_startFit = startSegment.getEndPerpS(startFit);

  m_endSegment_startPerpS_startFit = startSegment.getStartPerpS(startFit);
  m_endSegment_endPerpS_startFit = startSegment.getEndPerpS(startFit);

  m_startSegment_startPerpS_endFit = startSegment.getStartPerpS(endFit);
  m_startSegment_endPerpS_endFit = startSegment.getEndPerpS(endFit);

  m_endSegment_startPerpS_endFit = startSegment.getStartPerpS(endFit);
  m_endSegment_endPerpS_endFit = startSegment.getEndPerpS(endFit);

  Vector2D startCOM = startSegment.getCenterOfMass2D();
  Vector2D endCOM = endSegment.getCenterOfMass2D();

  m_startSegment_com = startCOM;
  m_endSegment_com = endCOM;

  FloatType startAbsMom = startFit.getAbsMom2D();
  FloatType endAbsMom = endFit.getAbsMom2D();

  m_startSegment_mom = startAbsMom;
  m_endSegment_mom = endAbsMom;

  Vector2D startCenter = startFit.getClosest(startCOM);
  Vector2D endCenter = endFit.getClosest(endCOM);

  m_startSegment_center = startCenter;
  m_endSegment_center = endCenter;

  Vector2D startExtrapolatedToEndCenter = startFit.getClosest(endCenter);
  Vector2D endExtrapolatedToStartCenter = endFit.getClosest(startCenter);

  m_startSegment_extrapolation = startExtrapolatedToEndCenter;
  m_endSegment_extrapolation = endExtrapolatedToStartCenter;

  Vector2D startMomAtCenter = startFit.getUnitMom2D(startCenter);
  Vector2D endMomAtCenter = endFit.getUnitMom2D(endCenter);

  m_startSegment_centerMom = startMomAtCenter;
  m_endSegment_centerMom = endMomAtCenter;

  Vector2D startMomAtExtrapolation = startFit.getUnitMom2D(startExtrapolatedToEndCenter);
  Vector2D endMomAtExtrapolation = endFit.getUnitMom2D(endExtrapolatedToStartCenter);

  m_startSegment_extarpolationMom = startMomAtExtrapolation;
  m_endSegment_extrapolationMom = endMomAtExtrapolation;

  m_distanceAtStart = startCenter.distance(endExtrapolatedToStartCenter);
  m_distanceAtEnd = endCenter.distance(startExtrapolatedToEndCenter);

  m_momAngleDeviationAtStart = startMomAtCenter.angleWith(endMomAtExtrapolation);
  m_momAngleDeviationAtEnd = endMomAtCenter.angleWith(startMomAtExtrapolation);

  //make a cut - make this more sophisticated at some point
  //double cosDeviation = endCenter.cosWith(pointOnFromTrack);
  //double tolerance = cos(PI / 180);

  return true;
}
