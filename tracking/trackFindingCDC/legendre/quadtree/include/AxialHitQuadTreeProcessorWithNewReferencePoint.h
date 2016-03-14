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
    class AxialHitQuadTreeProcessorWithNewReferencePoint : public QuadTreeProcessorTemplate<float, float, TrackHit, 2, 2> {

    public:

      AxialHitQuadTreeProcessorWithNewReferencePoint(const ChildRanges& ranges, std::pair<double, double> referencePoint) :
        QuadTreeProcessorTemplate(0, ranges), m_referencePoint(referencePoint) { }

    private:

      inline bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};


      std::pair<double, double> m_referencePoint;

    public:

      void provideItemsSet(std::vector<TrackHit*>& itemsVector) override final
      {
        clear();

        std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        quadtreeItemsVector.reserve(itemsVector.size());
        for (TrackHit* item : itemsVector) {
          if (item->getHitUsage() != TrackHit::c_notUsed) continue;
          if (insertItemInNode(m_quadTree, item, 0, 0))
            quadtreeItemsVector.push_back(new ItemType(item));
        }
      }

      std::vector<TrackHit*> getAssignedHits()
      {
        std::vector<TrackHit*> itemsToReturn;
        itemsToReturn.reserve(m_quadTree->getNItems());

        for (ItemType* item : m_quadTree->getItemsVector()) {
          itemsToReturn.push_back(item->getPointer());
        }

        return itemsToReturn;
      }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      inline bool insertItemInNode(QuadTree* node, TrackHit* hit, unsigned int /*t_index*/, unsigned int /*r_index*/) const override final
      {
        float dist_1[2][2];
        float dist_2[2][2];

        std::tuple<double, double, double> confCoords = hit->performConformalTransformWithRespectToPoint(m_referencePoint.first,
                                                        m_referencePoint.second);


        float r_temp_min = std::get<0>(confCoords) * cos(node->getXMin())
                           + std::get<1>(confCoords) * sin(node->getXMin());
        float r_temp_max = std::get<0>(confCoords) * cos(node->getXMax())
                           + std::get<1>(confCoords) * sin(node->getXMax());

        float r_min1 = r_temp_min - std::get<2>(confCoords);
        float r_min2 = r_temp_min + std::get<2>(confCoords);
        float r_max1 = r_temp_max - std::get<2>(confCoords);
        float r_max2 = r_temp_max + std::get<2>(confCoords);

        float m_rMin = node->getYMin();
        float m_rMax = node->getYMax();

        dist_1[0][0] = m_rMin - r_min1;
        dist_1[0][1] = m_rMin - r_max1;
        dist_1[1][0] = m_rMax - r_min1;
        dist_1[1][1] = m_rMax - r_max1;

        dist_2[0][0] = m_rMin - r_min2;
        dist_2[0][1] = m_rMin - r_max2;
        dist_2[1][0] = m_rMax - r_min2;
        dist_2[1][1] = m_rMax - r_max2;


        return (! sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1]) or
                ! sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1]));
      }

    };

  }
}
