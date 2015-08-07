/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#pragma once
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for StereoTrackHits
     * The two axis are the inverse slope in z direction and the z0.
     * We loop over the z0 and calculate the z-slope with the x-y-z-position the recoHit has.
     *  */
    class StereoHitQuadTreeProcessor final : public QuadTreeProcessorTemplate<float, float, const CDCRecoHit3D, 2, 2> {

    public:
      StereoHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges,
                                 bool debugOutput = false) : QuadTreeProcessorTemplate(lastLevel, ranges, debugOutput, false)
      {
      }

      /**
       * Do only insert the hit into a node if the slope and z information calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, const CDCRecoHit3D* hit, unsigned int /*slope_index*/,
                            unsigned int /*z0_index*/) const override final
      {
        float dist[2][2];

        const float& radius = hit->getRecoPos2D().norm();
        const float& reconstructedZ = hit->getRecoZ();

        float inverseSlopeMin = node->getXMin();
        float inverseSlopeMax = node->getXMax();
        float zMin = node->getYMin();
        float zMax = node->getYMax();

        dist[0][0] = radius * inverseSlopeMin - reconstructedZ + zMin;
        dist[0][1] = radius * inverseSlopeMin - reconstructedZ + zMax;
        dist[1][0] = radius * inverseSlopeMax - reconstructedZ + zMin;
        dist[1][1] = radius * inverseSlopeMax - reconstructedZ + zMax;

        return !FastHough::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1]);
      }
    };


    /** A QuadTreeProcessor for RecoSegments */
    class QuadTreeProcessorSegments final : public QuadTreeProcessorTemplate<int, float, CDCRecoSegment2D, 2, 2> {

    public:
      /**
       * The conformalTransformationPostion is the point which we use as an origin for the conformal transformation.
       */
      QuadTreeProcessorSegments(unsigned char lastLevel, const ChildRanges& ranges, const Vector2D& conformalTransformationPosition,
                                bool debugOutput = false) :
        QuadTreeProcessorTemplate(lastLevel, ranges, debugOutput), m_origin(conformalTransformationPosition) {}

      /**
       * Insert only if the sinogram of the front or the back part of the segment goes through the node.
       * Also we can intersect the sinograms of the front and the back and get an intersection point, which should lay in the node also.
       */
      bool insertItemInNode(QuadTree* node, CDCRecoSegment2D* recoItem, unsigned int /*t_index*/,
                            unsigned int /*r_index*/) const override final
      {
        float dist[2][2];

        TrigonometricalLookupTable& lookupTable = TrigonometricalLookupTable::Instance();

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

        dist[0][0] = rMin - rHitFrontMin;
        dist[0][1] = rMin - rHitFrontMax;
        dist[1][0] = rMax - rHitFrontMin;
        dist[1][1] = rMax - rHitFrontMax;

        if (FastHough::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
          return false;
        }

        float rHitBackMin = conformalTransformBack.dot(trigonometryMin);
        float rHitBackMax = conformalTransformBack.dot(trigonometryMax);

        dist[0][0] = rMin - rHitBackMin;
        dist[0][1] = rMin - rHitBackMax;
        dist[1][0] = rMax - rHitBackMin;
        dist[1][1] = rMax - rHitBackMax;

        if (FastHough::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
          return false;
        }

        return true;
      }

    private:
      Vector2D m_origin; /**< The origin of the conformal transformation */
    };

  }
}
