/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCSegmentPair::CDCSegmentPair() :
  m_startSegment(nullptr),
  m_endSegment(nullptr)
{
}



CDCSegmentPair::CDCSegmentPair(const CDCRecoSegment2D* startSegment,
                               const CDCRecoSegment2D* endSegment):
  m_startSegment(startSegment),
  m_endSegment(endSegment)
{
  B2ASSERT("CDCSegmentPair initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentPair initialized with nullptr as end segment", endSegment);
}


CDCSegmentPair::CDCSegmentPair(const CDCRecoSegment2D* startSegment,
                               const CDCRecoSegment2D* endSegment,
                               const CDCTrajectory3D& trajectory3D) :
  m_startSegment(startSegment),
  m_endSegment(endSegment),
  m_trajectory3D(trajectory3D)
{
  B2ASSERT("CDCSegmentPair initialized with nullptr as start segment", startSegment);
  B2ASSERT("CDCSegmentPair initialized with nullptr as end segment", endSegment);
}


double CDCSegmentPair::computeDeltaPhiAtSuperLayerBound() const
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





double CDCSegmentPair::computeStartIsBeforeEndFitless() const
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



double CDCSegmentPair::computeEndIsAfterStartFitless() const
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


double CDCSegmentPair::computeIsCoalignedFitless() const
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
