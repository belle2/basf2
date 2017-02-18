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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessorWithNewReferencePoint : public QuadTreeProcessorTemplate<float, float, const CDCWireHit> {

    public:

      /**
       * Constructor.
       * @param ranges ranges of the initial QuadTree (only one instance without children created)
       * @referencePoint reference position.
       */
      AxialHitQuadTreeProcessorWithNewReferencePoint(const XYSpans& ranges,
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

      /// Provide const CDCWireHit to process.
      void provideItemsSet(std::vector<const CDCWireHit*>& itemsVector) override final
      {
        clear();

        std::vector<Item*>& quadtreeItemsVector = m_quadTree->getItems();
        quadtreeItemsVector.reserve(itemsVector.size());
        for (const CDCWireHit* item : itemsVector) {
          if (item->getAutomatonCell().hasTakenFlag() or item->getAutomatonCell().hasMaskedFlag()) continue;
          if (insertItemInNode(m_quadTree.get(), item)) {
            quadtreeItemsVector.push_back(new Item(item));
          }
        }
      }

      /**
       * @return Hits which belong to the QuadTree
       */
      std::vector<const CDCWireHit*> getAssignedHits()
      {
        std::vector<const CDCWireHit*> itemsToReturn;
        itemsToReturn.reserve(m_quadTree->getNItems());

        for (Item* item : m_quadTree->getItems()) {
          itemsToReturn.push_back(item->getPointer());
        }

        return itemsToReturn;
      }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      bool insertItemInNode(QuadTree* node, const CDCWireHit* wireHit) const override final
      {
        double l = wireHit->getRefDriftLength();
        Vector2D pos2D = wireHit->getRefPos2D() - m_referencePoint;
        double r2 = pos2D.normSquared() - l * l;

        using Quadlet = std::array< std::array<float, 2>, 2>;
        Quadlet distRight{};
        Quadlet distLeft{};

        // get top and bottom borders of the node
        float rMin = node->getYMin() * r2 / 2;
        float rMax = node->getYMax() * r2 / 2;

        // get left and right borders of the node
        const Vector2D& thetaVecMin = Vector2D::Phi(node->getXMin());
        const Vector2D& thetaVecMax = Vector2D::Phi(node->getXMax());

        float rHitMin = thetaVecMin.dot(pos2D);
        float rHitMax = thetaVecMax.dot(pos2D);

        // compute sinograms at the left and right borders of the node
        float rHitMinRight = rHitMin - l;
        float rHitMaxRight = rHitMax - l;

        float rHitMinLeft = rHitMin + l;
        float rHitMaxLeft = rHitMax + l;

        // Compute distance from the sinograms to bottom and top borders of the node
        distRight[0][0] = rMin - rHitMinRight;
        distRight[0][1] = rMin - rHitMaxRight;
        distRight[1][0] = rMax - rHitMinRight;
        distRight[1][1] = rMax - rHitMaxRight;

        distLeft[0][0] = rMin - rHitMinLeft;
        distLeft[0][1] = rMin - rHitMaxLeft;
        distLeft[1][0] = rMax - rHitMinLeft;
        distLeft[1][1] = rMax - rHitMaxLeft;

        // Compare distance signes from sinograms to the node
        // Check right
        if (not sameSign(distRight[0][0], distRight[0][1], distRight[1][0], distRight[1][1])) {
          return true;
        }

        // Check left
        if (not sameSign(distLeft[0][0], distLeft[0][1], distLeft[1][0], distLeft[1][1])) {
          return true;
        }

        // Not contained
        return false;
      }
    };
  }
}
