/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/BoxDivisionHoughTree.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class AHitPtr, class AInBoxAlgorithm, size_t divisionX, size_t divisionY>
    class SimpeBoxDivisionHoughTree : public BoxDivisionHoughTree<AHitPtr, typename AInBoxAlgorithm::HoughBox, divisionX, divisionY> {

    private:
      typedef BoxDivisionHoughTree<AHitPtr, typename AInBoxAlgorithm::HoughBox, divisionX, divisionY> Super;
      typedef typename AInBoxAlgorithm::HoughBox HoughBox;
    public:
      /// Constructor using the given maximal level.
      SimpeBoxDivisionHoughTree(const float maximumX, const float maximumY) : Super(0),
        m_maximumX(maximumX), m_maximumY(maximumY) {}

      void initialize()
      {
        const size_t nXBins = std::pow(divisionX, Super::getMaxLevel());
        const size_t nDiscreteXs = nXBins + 1;
        const auto xBins = linspace<float>(-m_maximumX, m_maximumX, nDiscreteXs);

        const size_t nYBins = std::pow(divisionY, Super::getMaxLevel());
        const size_t nDiscreteYs = nYBins + 1;
        const auto yBins = linspace<float>(-m_maximumY, m_maximumY, nDiscreteYs);

        Super::template assignArray<0>(xBins, 0);
        Super::template assignArray<1>(yBins, 0);

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

    private:
      float m_maximumX = 0;
      float m_maximumY = 0;
    };
  }
}
