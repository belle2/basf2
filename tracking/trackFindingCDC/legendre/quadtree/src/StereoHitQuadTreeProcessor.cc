/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/StereoHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoHitQuadTreeProcessor::insertItemInNode(QuadTree* node, const CDCRecoHit3D* hit, unsigned int /*slope_index*/,
                                                  unsigned int /*z0_index*/) const
{
  float dist[2][2];

  const float& perpS = hit->getArcLength2D();
  const float& reconstructedZ = hit->getRecoZ();

  float inverseSlopeMin = node->getXMin();
  float inverseSlopeMax = node->getXMax();
  float zMin = node->getYMin();
  float zMax = node->getYMax();

  dist[0][0] = perpS * inverseSlopeMin - reconstructedZ + zMin;
  dist[0][1] = perpS * inverseSlopeMin - reconstructedZ + zMax;
  dist[1][0] = perpS * inverseSlopeMax - reconstructedZ + zMin;
  dist[1][1] = perpS * inverseSlopeMax - reconstructedZ + zMax;

  return !FastHough::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1]);
}
