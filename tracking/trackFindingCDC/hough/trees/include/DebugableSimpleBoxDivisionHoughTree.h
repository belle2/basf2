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
#include <tracking/trackFindingCDC/hough/trees/SimpleBoxDivisionHoughTree.h>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A convenience class for adding debug information to a Simple Hough Tree.
    template<class AHitPtr, class AInBoxAlgorithm, size_t divisionX, size_t divisionY>
    class DebugableSimpleBoxDivisionHoughTree :
      public SimpleBoxDivisionHoughTree<AHitPtr, AInBoxAlgorithm, divisionX, divisionY> {
    private:
      /// The Super class
      using Super = SimpleBoxDivisionHoughTree<AHitPtr, AInBoxAlgorithm, divisionX, divisionY>;
    public:
      /// Use the constructors of the super class.
      using Super::Super;

      /**
       *  Write out some debug information to a ROOT file with the given name.
       *  This must be done before felling the tree. Attention: This will delete a ROOT file
       *  with the same name if already present.
       *  @param filename The ROOT filename.
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

        double lowerLimX = -Super::getMaximumX();
        double upperLimX = Super::getMaximumX();
        double lowerLimY = -Super::getMaximumY();
        double upperLimY = Super::getMaximumY();
        double maxLevel = Super::getMaxLevel();
        eventTTree.Branch("lowerLimX", &lowerLimX);
        eventTTree.Branch("upperLimX", &upperLimX);
        eventTTree.Branch("lowerLimY", &lowerLimY);
        eventTTree.Branch("upperLimY", &upperLimY);
        eventTTree.Branch("maxLevel", &maxLevel);
        eventTTree.Fill();
        eventTTree.Write();

        auto walker = [&](const typename Super::Node * node) -> bool {
          // cppcheck-suppress unreadVariable
          lowerX = node->getLowerX();
          // cppcheck-suppress unreadVariable
          upperX = node->getUpperX();
          // cppcheck-suppress unreadVariable
          lowerY = node->getLowerY();
          // cppcheck-suppress unreadVariable
          upperY = node->getUpperY();
          // cppcheck-suppress unreadVariable
          weight = node->getWeight();
          // cppcheck-suppress unreadVariable
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
      /**
       *  Fill the tree till all nodes are touched once.
       *  This is not for finding results, but for debug reasons.
       */
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
