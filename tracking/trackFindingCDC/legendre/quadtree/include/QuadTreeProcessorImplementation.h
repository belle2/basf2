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
    class TrackHit;

    /** A QuadTreeProcessor for TrackHits */
    class HitQuadTreeProcessor : public QuadTreeProcessorTemplate<int, float, TrackHit, 2, 2> {

    public:

      HitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges) : QuadTreeProcessorTemplate(lastLevel, ranges) { }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, TrackHit* hit, unsigned int /*t_index*/, unsigned int /*r_index*/) const override
      {
        float dist_1[2][2];
        float dist_2[2][2];

        int thetaMin = node->getXMin();
        int thetaMax = node->getXMax();
        float rMin = node->getYMin();
        float rMax = node->getYMax();
        float cosThetaMin = TrigonometricalLookupTable::Instance().cosTheta(thetaMin);
        float sinThetaMin = TrigonometricalLookupTable::Instance().sinTheta(thetaMin);
        float cosThetaMax = TrigonometricalLookupTable::Instance().cosTheta(thetaMax);
        float sinThetaMax = TrigonometricalLookupTable::Instance().sinTheta(thetaMax);

        float rHitMin = hit->getConformalX() * cosThetaMin + hit->getConformalY() * sinThetaMin;
        float rHitMax = hit->getConformalX() * cosThetaMax + hit->getConformalY() * sinThetaMax;

        float rHitMin1 = rHitMin - hit->getConformalDriftLength();
        float rHitMin2 = rHitMin + hit->getConformalDriftLength();
        float rHitMax1 = rHitMax - hit->getConformalDriftLength();
        float rHitMax2 = rHitMax + hit->getConformalDriftLength();

        dist_1[0][0] = rMin - rHitMin1;
        dist_1[0][1] = rMin - rHitMax1;
        dist_1[1][0] = rMax - rHitMin1;
        dist_1[1][1] = rMax - rHitMax1;

        dist_2[0][0] = rMin - rHitMin2;
        dist_2[0][1] = rMin - rHitMax2;
        dist_2[1][0] = rMax - rHitMin2;
        dist_2[1][1] = rMax - rHitMax2;


        //curves are assumed to be straight lines, might be a reasonable assumption locally
        return (!FastHough::sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1]) or
                !FastHough::sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1]));
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       */
      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override
      {
        if ((node->getLevel() > (getLastLevel() - 7)) && (fabs(node->getYMean()) > 0.005)) {
          if (node->getLevel() < (getLastLevel() - 5)) {
            float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
            float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 4.;
            int theta1 = node->getXBin(i) - std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 4.);
            int theta2 = node->getXBin(i + 1) + std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 4.);

            if (theta1 < 0)
              theta1 = node->getXBin(i);
            if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
              theta2 = node->getXBin(i + 1);

            return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
          } else {
            float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 8.;
            float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 8.;
            int theta1 = node->getXBin(i) - std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 8.);
            int theta2 = node->getXBin(i + 1) + std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 8.);

            if (theta1 < 0)
              theta1 = node->getXBin(i);
            if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
              theta2 = node->getXBin(i + 1);

            return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
          }
        } else {
          return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
        }
      }
    };

    /** A QuadTreeProcessor for StereoTrackHits
     * The two axis are the inverse slope in z direction and the z0.
     * We loop over the z0 and calculate the z-slope with the x-y-z-position the recoHit has.
     *  */
    class StereoHitQuadTreeProcessor : public QuadTreeProcessorTemplate<float, float, const CDCRecoHit3D, 2, 2> {

    public:
      StereoHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges,
                                 bool debugOutput = false) : QuadTreeProcessorTemplate(lastLevel, ranges, debugOutput, false)
      {

        B2INFO(ranges.first.first << " " << ranges.first.second << " " << ranges.second.first << " " << ranges.second.second)

      }

      /**
       * Do only insert the hit into a node if the slope and z information calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, const CDCRecoHit3D* hit, unsigned int /*slope_index*/,
                            unsigned int /*z0_index*/) const override
      {
        float dist[2][2];

        float inverseSlopeMin = node->getXMin();
        float inverseSlopeMax = node->getXMax();
        float zMin = node->getYMin();
        float zMax = node->getYMax();

        float inverseSlopeForMinimum = 1 / hit->calculateZSlopeWithZ0(zMin);
        float inverseSlopeForMaximum = 1 / hit->calculateZSlopeWithZ0(zMax);

        dist[0][0] = inverseSlopeMin - inverseSlopeForMinimum;
        dist[0][1] = inverseSlopeMin - inverseSlopeForMaximum;
        dist[1][0] = inverseSlopeMax - inverseSlopeForMinimum;
        dist[1][1] = inverseSlopeMax - inverseSlopeForMaximum;

        return !FastHough::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1]);
      }
    };

    /** A QuadTreeProcessor for RecoSegments */
    class QuadTreeProcessorSegments : public QuadTreeProcessorTemplate<int, float, CDCRecoSegment2D, 2, 2> {

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
      bool insertItemInNode(QuadTree* node, CDCRecoSegment2D* recoItem, unsigned int /*t_index*/, unsigned int /*r_index*/) const override
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
