/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class TrackHit;

    class HitQuadTreeProcessor : public QuadTreeProcessorTemplate<int, float, TrackHit, 2, 2> {

    public:

      HitQuadTreeProcessor(unsigned int lastLevel) : QuadTreeProcessorTemplate(lastLevel) { }

      bool insertItemInNode(QuadTree* node, TrackHit* hit, unsigned int t_index, unsigned int r_index) const override
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

    class QuadTreeProcessorSegments : public QuadTreeProcessorTemplate<int, float, CDCRecoSegment2D, 2, 2> {

    public:

      QuadTreeProcessorSegments(unsigned int lastLevel) : QuadTreeProcessorTemplate(lastLevel) { }

      bool insertItemInNode(QuadTree* node, CDCRecoSegment2D* segment, unsigned int t_index, unsigned int r_index) const override
      {
        float distFront[2][2];
        float distBack[2][2];

        TrigonometricalLookupTable& lookupTable = TrigonometricalLookupTable::Instance();

        int thetaMin = node->getXMin();
        int thetaMax = node->getXMax();
        float rMin = node->getYMin();
        float rMax = node->getYMax();

        Vector2D trigonometryMin(lookupTable.cosTheta(thetaMin), lookupTable.sinTheta(thetaMin));
        Vector2D trigonometryMax(lookupTable.cosTheta(thetaMax), lookupTable.sinTheta(thetaMax));

        Vector2D conformalTransformFront = segment->front().getRecoPos2D().conformalTransformed();
        Vector2D conformalTransformBack = segment->back().getRecoPos2D().conformalTransformed();

        float rHitFrontMin = conformalTransformFront.dot(trigonometryMin);
        float rHitFrontMax = conformalTransformFront.dot(trigonometryMax);

        distFront[0][0] = rMin - rHitFrontMin;
        distFront[0][1] = rMin - rHitFrontMax;
        distFront[1][0] = rMax - rHitFrontMin;
        distFront[1][1] = rMax - rHitFrontMax;

        float rHitBackMin = conformalTransformBack.dot(trigonometryMin);
        float rHitBackMax = conformalTransformBack.dot(trigonometryMax);

        distBack[0][0] = rMin - rHitBackMin;
        distBack[0][1] = rMin - rHitBackMax;
        distBack[1][0] = rMax - rHitBackMin;
        distBack[1][1] = rMax - rHitBackMax;

        //curves are assumed to be straight lines, might be a reasonable assumption locally
        return (!FastHough::sameSign(distFront[0][0], distFront[0][1], distFront[1][0], distFront[1][1]) or
                !FastHough::sameSign(distBack[0][0], distBack[0][1], distBack[1][0], distBack[1][1]));
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

  }
}
