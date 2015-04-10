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
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class TrackHit;

    /** A QuadTreeProcessor for TrackHits */
    class HitQuadTreeProcessor : public QuadTreeProcessorTemplate<int, float, TrackHit, 2, 2> {

    public:

      using QuadTreeProcessorTemplate::QuadTreeProcessorTemplate;

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
       * Return
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

    template <class type>
    class QuadTreeProcessorReco : public QuadTreeProcessorTemplate<int, float, type, 2, 2> {

    public:
      using typename QuadTreeProcessorTemplate<int, float, type, 2, 2>::QuadTree;  /**< The used QuadTree */
      using typename QuadTreeProcessorTemplate<int, float, type, 2, 2>::rangeX;   /**< This pair describes the range in X for a node */
      using typename QuadTreeProcessorTemplate<int, float, type, 2, 2>::rangeY;   /**< This pair describes the range in Y for a node */
      using typename
      QuadTreeProcessorTemplate<int, float, type, 2, 2>::ChildRanges; /**< This pair of ranges describes the range of a node */

      using QuadTreeProcessorTemplate<int, float, type, 2, 2>::QuadTreeProcessorTemplate;

      bool insertItemInNode(QuadTree* node, type* recoItem, unsigned int /*t_index*/, unsigned int /*r_index*/) const override
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

        Vector2D conformalTransformFront = recoItem->front().getRecoPos2D().conformalTransformed();
        Vector2D conformalTransformBack = recoItem->back().getRecoPos2D().conformalTransformed();

        double thetaIntersection = std::atan2((conformalTransformBack - conformalTransformFront).x(),
                                              (conformalTransformFront - conformalTransformBack).y());

        if (thetaIntersection < 0) {
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

      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override
      {
        int thetaMin = node->getXBin(i);
        int thetaMax = node->getXBin(i + 1);
        float rMin = node->getYBin(j);
        float rMax = node->getYBin(j + 1);
        return ChildRanges(rangeX(thetaMin, thetaMax), rangeY(rMin, rMax));
      }
    };

    typedef QuadTreeProcessorReco<CDCRecoSegment2D> QuadTreeProcessorSegments;

  }
}
