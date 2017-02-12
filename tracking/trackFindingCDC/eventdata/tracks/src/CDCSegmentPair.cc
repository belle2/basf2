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

using namespace Belle2;
using namespace TrackFindingCDC;


CDCSegmentPair::CDCSegmentPair() :
  m_fromSegment(nullptr),
  m_toSegment(nullptr)
{
}



CDCSegmentPair::CDCSegmentPair(const CDCSegment2D* fromSegment,
                               const CDCSegment2D* toSegment):
  m_fromSegment(fromSegment),
  m_toSegment(toSegment)
{
  B2ASSERT("CDCSegmentPair initialized with nullptr as from segment", fromSegment);
  B2ASSERT("CDCSegmentPair initialized with nullptr as to segment", toSegment);
}


CDCSegmentPair::CDCSegmentPair(const CDCSegment2D* fromSegment,
                               const CDCSegment2D* toSegment,
                               const CDCTrajectory3D& trajectory3D) :
  m_fromSegment(fromSegment),
  m_toSegment(toSegment),
  m_trajectory3D(trajectory3D)
{
  B2ASSERT("CDCSegmentPair initialized with nullptr as from segment", fromSegment);
  B2ASSERT("CDCSegmentPair initialized with nullptr as to segment", toSegment);
}


double CDCSegmentPair::computeDeltaPhiAtSuperLayerBound() const
{
  const CDCSegment2D* ptrFromSegment = getFromSegment();
  const CDCSegment2D* ptrToSegment = getToSegment();

  if (not ptrFromSegment) {
    return NAN;
  }

  if (not ptrToSegment) {
    return NAN;
  }

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  if (fromSegment.empty() or toSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& lastRecoHit_fromSegment = fromSegment.back();
  const CDCRecoHit2D& firstRecoHit_toSegment = toSegment.front();

  const Vector2D lastPos2D_fromSegment =  lastRecoHit_fromSegment.getRecoPos2D();
  const Vector2D firstPos2D_toSegment =  firstRecoHit_toSegment.getRecoPos2D();

  return lastPos2D_fromSegment.angleWith(firstPos2D_toSegment);
}





double CDCSegmentPair::computeFromIsBeforeToFitless() const
{
  const CDCSegment2D* ptrFromSegment = getFromSegment();
  const CDCSegment2D* ptrToSegment = getToSegment();

  if (not ptrFromSegment) {
    return NAN;
  }

  if (not ptrToSegment) {
    return NAN;
  }

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  if (fromSegment.empty() or toSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& firstRecoHit_fromSegment = fromSegment.front();
  const CDCRecoHit2D& lastRecoHit_fromSegment = fromSegment.back();

  const CDCRecoHit2D& firstRecoHit_toSegment = toSegment.front();

  const Vector2D firstPos2D_fromSegment =  firstRecoHit_fromSegment.getRecoPos2D();
  const Vector2D lastPos2D_fromSegment =  lastRecoHit_fromSegment.getRecoPos2D();
  const Vector2D firstPos2D_toSegment =  firstRecoHit_toSegment.getRecoPos2D();

  Vector2D firstToLast_fromSegment = lastPos2D_fromSegment - firstPos2D_fromSegment;
  Vector2D firstToFirst = firstPos2D_toSegment - firstPos2D_fromSegment;

  return firstToLast_fromSegment.angleWith(firstToFirst);

}



double CDCSegmentPair::computeToIsAfterFromFitless() const
{
  const CDCSegment2D* ptrFromSegment = getFromSegment();
  const CDCSegment2D* ptrToSegment = getToSegment();

  if (not ptrFromSegment) {
    return NAN;
  }

  if (not ptrToSegment) {
    return NAN;
  }

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  if (fromSegment.empty() or toSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& lastRecoHit_fromSegment = fromSegment.back();

  const CDCRecoHit2D& firstRecoHit_toSegment = toSegment.front();
  const CDCRecoHit2D& lastRecoHit_toSegment = toSegment.back();

  const Vector2D lastPos2D_fromSegment =  lastRecoHit_fromSegment.getRecoPos2D();
  const Vector2D firstPos2D_toSegment =  firstRecoHit_toSegment.getRecoPos2D();
  const Vector2D lastPos2D_toSegment =  lastRecoHit_toSegment.getRecoPos2D();

  Vector2D firstToLast_toSegment = lastPos2D_toSegment - firstPos2D_toSegment;
  Vector2D lastToLast = lastPos2D_toSegment - lastPos2D_fromSegment;

  return firstToLast_toSegment.angleWith(lastToLast);

}


double CDCSegmentPair::computeIsCoalignedFitless() const
{
  const CDCSegment2D* ptrFromSegment = getFromSegment();
  const CDCSegment2D* ptrToSegment = getToSegment();

  if (not ptrFromSegment) {
    return NAN;
  }

  if (not ptrToSegment) {
    return NAN;
  }


  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  if (fromSegment.empty() or toSegment.empty()) {
    return NAN;
  }

  const CDCRecoHit2D& firstRecoHit_fromSegment = fromSegment.front();
  const CDCRecoHit2D& lastRecoHit_fromSegment = fromSegment.back();

  const CDCRecoHit2D& firstRecoHit_toSegment = toSegment.front();
  const CDCRecoHit2D& lastRecoHit_toSegment = toSegment.back();

  const Vector2D firstPos2D_fromSegment =  firstRecoHit_fromSegment.getRecoPos2D();
  const Vector2D lastPos2D_fromSegment =  lastRecoHit_fromSegment.getRecoPos2D();

  const Vector2D firstPos2D_toSegment =  firstRecoHit_toSegment.getRecoPos2D();
  const Vector2D lastPos2D_toSegment =  lastRecoHit_toSegment.getRecoPos2D();


  Vector2D firstToLast_fromSegment = lastPos2D_fromSegment - firstPos2D_fromSegment;
  Vector2D firstToLast_toSegment = lastPos2D_toSegment - firstPos2D_toSegment;

  return firstToLast_fromSegment.angleWith(firstToLast_toSegment);
}
