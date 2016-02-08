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
#include <TTree.h>
#include <TFile.h>

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

      /// Return the maximum value in x direction.
      const float& getMaximumX() const
      {
        return m_maximumX;
      }

      /// Return the maximum value in y direction.
      const float& getMaximumY() const
      {
        return m_maximumY;
      }

      /// Write out some debug information to a ROOT file with the given name.
      /**
       * This must be done before felling the tree. Attention: This will delete a ROOT file
       * with the same name if already present.
       * @param filename: The ROOT filename.
       */
      void writeDebugInfoToFile(const std::string& filename)
      {
        fillAll();

        TFile openedRootFile(filename.c_str(), "RECREATE");
        TTree weightTTree("weightTree", "A tree with the weights of the box items.");
        TTree eventTTree("eventTree", "A tree with event information.");

        double lowerX, upperX, lowerY, upperY, weight, level;
        weightTTree.Branch("lowerX", &lowerX);
        weightTTree.Branch("upperY", &upperY);
        weightTTree.Branch("lowerY", &lowerY);
        weightTTree.Branch("upperX", &upperX);
        weightTTree.Branch("weight", &weight);
        weightTTree.Branch("level", &level);

        double lowerLimX = -getMaximumX();
        double upperLimX = getMaximumX();
        double lowerLimY = -getMaximumY();
        double upperLimY = getMaximumY();
        double maxLevel = Super::getMaxLevel();
        eventTTree.Branch("lowerLimX", &lowerLimX);
        eventTTree.Branch("upperLimX", &upperLimX);
        eventTTree.Branch("lowerLimY", &lowerLimY);
        eventTTree.Branch("upperLimY", &upperLimY);
        eventTTree.Branch("maxLevel", &maxLevel);
        eventTTree.Fill();
        eventTTree.Write();

        auto walker = [&](const typename Super::Node * node) -> bool {
          lowerX = node->getLowerZ0();
          upperX = node->getUpperZ0();
          lowerY = node->getLowerTanLambda();
          upperY = node->getUpperTanLambda();
          weight = node->getWeight();
          level = node->getLevel();


          weightTTree.Fill();

          // Always return true, as we want to access every node
          return true;
        };
        Super::getTree()->walk(walker);

        weightTTree.Write();
        openedRootFile.Write();
        openedRootFile.Close();
      }

    private:
      float m_maximumX = 0;
      float m_maximumY = 0;

      /// Fill the tree till all nodes are touched once.
      /** This is not for finding results, but for debug reasons. **/
      void fillAll()
      {
        AInBoxAlgorithm inBoxAlgorithm;

        auto isLeaf = [this](typename Super::Node * node) {
          return (node->getLevel() > this->getMaxLevel());
        };

        this->getTree()->fillWalk(inBoxAlgorithm, isLeaf);
      }
    };
  }
}
