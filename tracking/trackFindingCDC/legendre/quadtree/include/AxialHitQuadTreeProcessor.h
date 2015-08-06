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
    class AxialHitQuadTreeProcessor : public QuadTreeProcessorTemplate<int, float, TrackHit, 2, 2> {

    public:

      AxialHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges) : QuadTreeProcessorTemplate(lastLevel, ranges) { }

    private:

      inline bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

    public:

      /**
       * lastLevel depends on curvature of the track candidate
       */
      bool checkIfLastLevel(QuadTree* node) override final
      {
        unsigned char level_diff = 0;
        if (fabs(node->getYMean()) > 0.07) level_diff = 2;
        else if ((fabs(node->getYMean()) < 0.07) && (fabs(node->getYMean()) > 0.04))
          level_diff = 1;
        if (node->getLevel() >= (getLastLevel() - level_diff)) return true;
        else return false;
      }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      inline bool insertItemInNode(QuadTree* node, TrackHit* hit, unsigned int /*t_index*/, unsigned int /*r_index*/) const override final
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
        return (! sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1]) or
                ! sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1]));
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       */
      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override final
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

  }
}
