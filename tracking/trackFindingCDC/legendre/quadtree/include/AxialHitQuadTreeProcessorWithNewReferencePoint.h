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
#include <tracking/trackFindingCDC/eventdata/hits/CDCConformalHit.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessorWithNewReferencePoint : public QuadTreeProcessorTemplate<float, float, CDCConformalHit, 2, 2> {

    public:

      /**
       * Constructor.
       * @param ranges ranges of the initial QuadTree (only one instance without children created)
       * @referencePoint reference position. Conformal transformation will be performed with respect to this point rather than to (0;0)
       */
      AxialHitQuadTreeProcessorWithNewReferencePoint(const ChildRanges& ranges,
                                                     std::pair<double, double> referencePoint)
        : QuadTreeProcessorTemplate(0, ranges),
          m_referencePoint(referencePoint.first, referencePoint.second)
      {}

    private:

      /// Function to check whether sinogram is crossing the node (see AxialHitQuadTreeProcessorWithNewReferencePoint::insertItemInNode())
      bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

      /// Reference point
      Vector2D m_referencePoint;

    public:

      /// Provide CDCConformalHit to process.
      void provideItemsSet(std::vector<CDCConformalHit*>& itemsVector) override final
      {
        clear();

        std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        quadtreeItemsVector.reserve(itemsVector.size());
        for (CDCConformalHit* item : itemsVector) {
          if (item->getUsedFlag()) continue;
          if (insertItemInNode(m_quadTree, item))
            quadtreeItemsVector.push_back(new ItemType(item));
        }
      }

      /**
       * @return Hits which belong to the QuadTree
       */
      std::vector<CDCConformalHit*> getAssignedHits()
      {
        std::vector<CDCConformalHit*> itemsToReturn;
        itemsToReturn.reserve(m_quadTree->getNItems());

        for (ItemType* item : m_quadTree->getItemsVector()) {
          itemsToReturn.push_back(item->getPointer());
        }

        return itemsToReturn;
      }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, CDCConformalHit* hit) const override final
      {
        float dist_1[2][2];
        float dist_2[2][2];

        std::tuple<Vector2D, double> confCoords = hit->performConformalTransformWithRespectToPoint(m_referencePoint);
        Vector2D min_n12 = Vector2D::Phi(node->getXMin());
        Vector2D max_n12 = Vector2D::Phi(node->getXMax());

        float r_temp_min = std::get<0>(confCoords).dot(min_n12);
        float r_temp_max = std::get<0>(confCoords).dot(max_n12);

        float r_min1 = r_temp_min - std::get<1>(confCoords);
        float r_min2 = r_temp_min + std::get<1>(confCoords);
        float r_max1 = r_temp_max - std::get<1>(confCoords);
        float r_max2 = r_temp_max + std::get<1>(confCoords);

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
