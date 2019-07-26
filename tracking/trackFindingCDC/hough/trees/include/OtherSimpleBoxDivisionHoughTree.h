/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/BoxDivisionHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Convenience class for the typical usage-case: A box divisioned hough tree with maximum and minimum values in both directions.
    /// This time in 3D NOTE Z direction is positive only
    template<class AHitPtr, class AInBoxAlgorithm, size_t divisionX, size_t divisionY, size_t divisionZ>
    class OtherSimpleBoxDivisionHoughTree : public
      BoxDivisionHoughTree<AHitPtr, typename AInBoxAlgorithm::HoughBox, divisionX, divisionY, divisionZ> {

    private:
      /// The Super class.
      using Super = BoxDivisionHoughTree<AHitPtr, typename AInBoxAlgorithm::HoughBox, divisionX, divisionY, divisionZ>;

      /// The HoughBox we use.
      using HoughBox = typename AInBoxAlgorithm::HoughBox;

      /// Type of the width in coordinate I.
      template <size_t I>
      using Width = typename HoughBox::template Width<I>;

    public:
      /// Constructor using the given maximal level.
      OtherSimpleBoxDivisionHoughTree(float maximumX,
                                      float maximumY,
                                      float maximumZ,
                                      Width<0> overlapX = 0,
                                      Width<1> overlapY = 0,
                                      Width<2> overlapZ = 0)
        : Super(0)
        , m_maximumX(maximumX)
        , m_maximumY(maximumY)
        , m_maximumZ(maximumZ)
        , m_overlapX(overlapX)
        , m_overlapY(overlapY)
        , m_overlapZ(overlapZ)
      {
      }

      /// Initialize the tree with the given values.
      void initialize()
      {
        Super::template constructArray<0>(-getMaximumX(), getMaximumX(), getOverlapX());
        Super::template constructArray<1>(0.01, getMaximumY(), getOverlapY());
        Super::template constructArray<2>(0.01, getMaximumZ(), getOverlapZ());

        Super::initialize();
      }

      /// Find only the leave with the highest weight = number of items
      std::vector<std::pair<HoughBox, std::vector<AHitPtr>>>
      findSingleBest(const Weight& minWeight)
      {
        AInBoxAlgorithm inBoxAlgorithm;
        auto skipLowWeightNode = [minWeight](const typename Super::Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        auto found = this->getTree()->findHeaviestLeafSingle(inBoxAlgorithm, this->getMaxLevel(), skipLowWeightNode);

        std::vector<std::pair<HoughBox, std::vector<AHitPtr>>> result;
        if (found) {
          // Move the found content over. unique_ptr still destroys the left overs.
          result.push_back(std::move(*found));
        }
        return result;
      }

      void writeDebugInfoToFile(const std::string& filename __attribute__((unused)))
      {
        //do nothing;
      }

      void drawDebugPlot(const std::vector<CDCRecoHit3D>& allHits __attribute__((unused)),
                         const std::vector<CDCRecoHit3D>& foundHits __attribute__((unused)),
                         const HoughBox& node __attribute__((unused)))
      {
        //do nothing;
      }

      /// Return the maximum value in x direction.
      float getMaximumX() const
      {
        return m_maximumX;
      }

      /// Return the maximum value in y direction.
      float getMaximumY() const
      {
        return m_maximumY;
      }

      /// Return the maximum value in Z direction.
      float getMaximumZ() const
      {
        return m_maximumZ;
      }

      /// Return the overlap in x direction.
      Width<0> getOverlapX() const
      {
        return m_overlapX;
      }

      /// Return the overlap in y direction.
      Width<1> getOverlapY() const
      {
        return m_overlapY;
      }

      /// Return the overlap in y direction.
      Width<2> getOverlapZ() const
      {
        return m_overlapZ;
      }

    private:
      /// The maximum value in X direction.
      float m_maximumX = 0;

      /// The maximum value in y direction.
      float m_maximumY = 0;

      /// The maximum value in z direction.
      float m_maximumZ = 0;

      /// The overlap in X direction.
      Width<0> m_overlapX = 0;

      /// The overlap in Y direction.
      Width<1> m_overlapY = 0;

      /// The overlap in Y direction.
      Width<2> m_overlapZ = 0;
    };
  }
}
