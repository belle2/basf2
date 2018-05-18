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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <TGraph.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TAxis.h>

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
      // In all use cases for now AHitPtr is an std::pair< CDCRecoHit3D, ... >
      using Hit = typename AHitPtr::first_type;

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

      void drawDebugPlots(std::vector<const Hit&> allHits, std::vector<const Hit&> foundHits, const typename Super::Node* node)
      {
        TGraph* allHitsGraph = new TGraph();
        allHitsGraph->SetLineWidth(2);
        allHitsGraph->SetLineColor(9);

        for (const CDCRecoHit3D& recoHit3D : allHits) {
          const Vector3D& recoPos3D = recoHit3D.getRecoPos3D();
          const double R = std::sqrt(recoPos3D.x() * recoPos3D.x() + recoPos3D.y() * recoPos3D.y());
          const double Z = recoPos3D.z();
          allHitsGraph->SetPoint(allHitsGraph->GetN(), R, Z);
        }

        static int nevent(0);
        TCanvas canv("trackCanvas", "CDC stereo hits in an event", 0, 0, 1600, 1200);
        canv.cd();
        allHitsGraph->Draw("APL*");
        allHitsGraph->GetXaxis()->SetLimits(0, 120);
        allHitsGraph->GetYaxis()->SetRangeUser(-180, 180);

        TGraph* foundHitsGraph = new TGraph();
        foundHitsGraph->SetMarkerStyle(8);
        foundHitsGraph->SetMarkerColor(2);

        for (const CDCRecoHit3D& recoHit3D : foundHits) {
          const Vector3D& recoPos3D = recoHit3D.getRecoPos3D();
          const double R = std::sqrt(recoPos3D.x() * recoPos3D.x() + recoPos3D.y() * recoPos3D.y());
          const double Z = recoPos3D.z();
          foundHitsGraph->SetPoint(foundHitsGraph->GetN(), R, Z);
        }
        foundHitsGraph->Draw("P");

        const double xMean = (node.getLowerX() + node.getUpperX()) / 2.0; //Z0 or Z1
        const double yMean = (node.getLowerY() + node.getUpperY()) / 2.0; //tanLambda or Z2
        const double xLow = node.getLowerX();
        const double yLow = node.getLowerY();
        const double xHigh = node.getUpperX();
        const double yHigh = node.getUpperY();

        TF1* candidateLL = new TF1("candLL", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateLH = new TF1("candLH", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateHL = new TF1("candHL", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateHH = new TF1("candHH", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateMean = new TF1("candMean", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);

        candidateLL->SetParameters(xLow, yLow);
        candidateLH->SetParameters(xLow, yHigh);
        candidateHL->SetParameters(xHigh, yLow);
        candidateHH->SetParameters(xHigh, yHigh);
        candidateMean->SetParameters(xMean, yMean);

        candidateLL->SetLineColor(9);
        candidateLH->SetLineColor(30);
        candidateHL->SetLineColor(46);
        candidateHH->SetLineColor(41);
        candidateMean->SetLineColor(2);

        candidateLL->Draw("same");
        candidateHL->Draw("same");
        candidateLH->Draw("same");
        candidateHH->Draw("same");
        candidateMean->Draw("same");
        canv.SaveAs(Form("CDCRLHits_%i.png", nevent));
        nevent++;
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
