/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCAxialStereoSegmentPair.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCAxialStereoSegmentPair)



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair() : m_startSegment(nullptr),  m_endSegment(nullptr)
{

}



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                                     const CDCAxialRecoSegment2D* endSegment):
  m_startSegment(startSegment),
  m_endSegment(endSegment)
{
  if (not startSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as end segment");
}



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                                     const CDCAxialRecoSegment2D* endSegment,
                                                     const CDCTrajectory3D& trajectory3D) :
  m_startSegment(startSegment),
  m_endSegment(endSegment),
  m_trajectory3D(trajectory3D)
{
  if (not startSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as end segment");
}



CDCAxialStereoSegmentPair::~CDCAxialStereoSegmentPair()
{
}



FloatType CDCAxialStereoSegmentPair::computeDeltaPhiAtSuperLayerBound() const
{
  const CDCRecoSegment2D* ptrStartSegment = getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = getEndSegment();

  if (not ptrStartSegment) {
    return NAN;
  }

  if (not ptrEndSegment) {
    return NAN;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  if (startSegment.empty() or endSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& lastRecoHit_startSegment = startSegment.back();
  const CDCRecoHit2D& firstRecoHit_endSegment = endSegment.front();

  const Vector2D lastPos2D_startSegment =  lastRecoHit_startSegment.getRecoPos2D();
  const Vector2D firstPos2D_endSegment =  firstRecoHit_endSegment.getRecoPos2D();

  return lastPos2D_startSegment.angleWith(firstPos2D_endSegment);
}





FloatType CDCAxialStereoSegmentPair::computeStartIsBeforeEndFitless() const
{
  const CDCRecoSegment2D* ptrStartSegment = getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = getEndSegment();

  if (not ptrStartSegment) {
    return NAN;
  }

  if (not ptrEndSegment) {
    return NAN;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  if (startSegment.empty() or endSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& firstRecoHit_startSegment = startSegment.front();
  const CDCRecoHit2D& lastRecoHit_startSegment = startSegment.back();

  const CDCRecoHit2D& firstRecoHit_endSegment = endSegment.front();

  const Vector2D firstPos2D_startSegment =  firstRecoHit_startSegment.getRecoPos2D();
  const Vector2D lastPos2D_startSegment =  lastRecoHit_startSegment.getRecoPos2D();
  const Vector2D firstPos2D_endSegment =  firstRecoHit_endSegment.getRecoPos2D();

  Vector2D firstToLast_startSegment = lastPos2D_startSegment - firstPos2D_startSegment;
  Vector2D firstToFirst = firstPos2D_endSegment - firstPos2D_startSegment;

  return firstToLast_startSegment.angleWith(firstToFirst);

}



FloatType CDCAxialStereoSegmentPair::computeEndIsAfterStartFitless() const
{
  const CDCRecoSegment2D* ptrStartSegment = getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = getEndSegment();

  if (not ptrStartSegment) {
    return NAN;
  }

  if (not ptrEndSegment) {
    return NAN;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  if (startSegment.empty() or endSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& lastRecoHit_startSegment = startSegment.back();

  const CDCRecoHit2D& firstRecoHit_endSegment = endSegment.front();
  const CDCRecoHit2D& lastRecoHit_endSegment = endSegment.back();

  const Vector2D lastPos2D_startSegment =  lastRecoHit_startSegment.getRecoPos2D();
  const Vector2D firstPos2D_endSegment =  firstRecoHit_endSegment.getRecoPos2D();
  const Vector2D lastPos2D_endSegment =  lastRecoHit_endSegment.getRecoPos2D();

  Vector2D firstToLast_endSegment = lastPos2D_endSegment - firstPos2D_endSegment;
  Vector2D lastToLast = lastPos2D_endSegment - lastPos2D_startSegment;

  return firstToLast_endSegment.angleWith(lastToLast);

}


FloatType CDCAxialStereoSegmentPair::computeIsCoalignedFitless() const
{
  const CDCRecoSegment2D* ptrStartSegment = getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = getEndSegment();

  if (not ptrStartSegment) {
    return NAN;
  }

  if (not ptrEndSegment) {
    return NAN;
  }


  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  if (startSegment.empty() or endSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& firstRecoHit_startSegment = startSegment.front();
  const CDCRecoHit2D& lastRecoHit_startSegment = startSegment.back();

  const CDCRecoHit2D& firstRecoHit_endSegment = endSegment.front();
  const CDCRecoHit2D& lastRecoHit_endSegment = endSegment.back();

  const Vector2D firstPos2D_startSegment =  firstRecoHit_startSegment.getRecoPos2D();
  const Vector2D lastPos2D_startSegment =  lastRecoHit_startSegment.getRecoPos2D();

  const Vector2D firstPos2D_endSegment =  firstRecoHit_endSegment.getRecoPos2D();
  const Vector2D lastPos2D_endSegment =  lastRecoHit_endSegment.getRecoPos2D();


  Vector2D firstToLast_startSegment = lastPos2D_startSegment - firstPos2D_startSegment;
  Vector2D firstToLast_endSegment = lastPos2D_endSegment - firstPos2D_endSegment;

  return firstToLast_startSegment.angleWith(firstToLast_endSegment);
}
