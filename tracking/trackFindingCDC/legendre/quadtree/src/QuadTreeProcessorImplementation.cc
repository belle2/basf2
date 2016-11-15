/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool QuadTreeProcessorSegments::insertItemInNode(QuadTree* node, CDCSegment2D* recoItem, unsigned int /*t_index*/,
                                                 unsigned int /*r_index*/) const
{
  float dist[2][2];

  TrigonometricalLookupTable<>& lookupTable = TrigonometricalLookupTable<>::Instance();

  float bin_width = TMath::Pi() / lookupTable.getNBinsTheta();

  int thetaIndexMin = node->getXMin();
  int thetaIndexMax = node->getXMax();
  float rMin = node->getYMin();
  float rMax = node->getYMax();

  float thetaMin = thetaIndexMin * bin_width;
  float thetaMax = thetaIndexMax * bin_width;

  Vector2D conformalTransformFront = recoItem->front().getRecoPos2D().subtract(m_origin).conformalTransformed();
  Vector2D conformalTransformBack = recoItem->back().getRecoPos2D().subtract(m_origin).conformalTransformed();

  double thetaIntersection = std::atan2((conformalTransformBack - conformalTransformFront).x(),
                                        (conformalTransformFront - conformalTransformBack).y());

  while (thetaIntersection < 0) {
    thetaIntersection += TMath::Pi();
  }

  if (thetaIntersection < thetaMin or thetaIntersection > thetaMax) {
    return false;
  }

  Vector2D trigonometryMin(lookupTable.cosTheta(thetaIndexMin), lookupTable.sinTheta(thetaIndexMin));
  Vector2D trigonometryMax(lookupTable.cosTheta(thetaIndexMax), lookupTable.sinTheta(thetaIndexMax));

  float rHitFrontMin = conformalTransformFront.dot(trigonometryMin);
  float rHitFrontMax = conformalTransformFront.dot(trigonometryMax);

  // cppcheck-suppress arrayIndexOutOfBounds
  dist[0][0] = rMin - rHitFrontMin;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[0][1] = rMin - rHitFrontMax;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[1][0] = rMax - rHitFrontMin;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[1][1] = rMax - rHitFrontMax;

  if (sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
    return false;
  }

  float rHitBackMin = conformalTransformBack.dot(trigonometryMin);
  float rHitBackMax = conformalTransformBack.dot(trigonometryMax);

  // cppcheck-suppress arrayIndexOutOfBounds
  dist[0][0] = rMin - rHitBackMin;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[0][1] = rMin - rHitBackMax;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[1][0] = rMax - rHitBackMin;
  // cppcheck-suppress arrayIndexOutOfBounds
  dist[1][1] = rMax - rHitBackMax;

  if (sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
    return false;
  }

  return true;
}
