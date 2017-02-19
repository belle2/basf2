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

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>

#include <array>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessor<long, float, const CDCWireHit> {

    public:
      /// Constructor
      AxialHitQuadTreeProcessor(unsigned char lastLevel,
                                const XYSpans& ranges,
                                PrecisionUtil::PrecisionFunction lmdFunctLevel,
                                bool standartBinning = false)
        : QuadTreeProcessor(lastLevel, ranges)
        , m_standartBinning(standartBinning)
        , m_lmdFunctLevel(lmdFunctLevel)
      {
      }

    private:

      /**
       *  Sets whether standard splitting of bins will be used
       *   - in case of standard binning each bin will be splitted into 4 equal bins
       *   - in case of non-standard binning boundaries of each child will be extended (see AxialHitQuadTreeProcessor::createChildWithParent())
       */
      bool m_standartBinning;

      /// Lambda which holds resolution function for the quadtree
      PrecisionUtil::PrecisionFunction m_lmdFunctLevel;

      /// Function to check whether sinogram is crossing the node (see AxialHitQuadTreeProcessor::insertItemInNode())
      bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

      /**
       * Vector of QuadTrees
       * QuadTree instances (which are filled in the vector) cover the whole Legendre phase-space; each instance is processes independently.
       */
      std::vector<QuadTree> m_seededTree;

    public:

      /**
       * lastLevel depends on curvature of the track candidate
       */
      bool isLeaf(QuadTree* node) const final
      {
        if (node->getLevel() <= 6) return false;
        if (node->getLevel() >= getLastLevel()) return true;

        double nodeResolution = fabs(node->getYMin() - node->getYMax());

        double resolution = m_lmdFunctLevel(node->getYMean());
        if (resolution >= nodeResolution) return true;

        return false;
      }

      /**
       * Check whether hit belongs to the quadtree node:
       * @param node quadtree node
       * @param hit hit being checked
       * @return returns true if sinogram of the hit crosses (geometrically) borders of the node
       */
      bool isInNode(QuadTree* node, const CDCWireHit* wireHit) const final
      {
        const double& l = wireHit->getRefDriftLength();
        const Vector2D& pos2D = wireHit->getRefPos2D();
        double r2 = square(wireHit->getRefCylindricalR()) - l * l;

        using Quadlet = std::array< std::array<float, 2>, 2>;
        Quadlet distRight{};
        Quadlet distLeft{};

        // get top and bottom borders of the node
        float rMin = node->getYMin() * r2 / 2;
        float rMax = node->getYMax() * r2 / 2;

        // get left and right borders of the node
        long thetaMin = node->getXMin();
        long thetaMax = node->getXMax();

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();
        const Vector2D& thetaVecMin = trigonometricalLookupTable.thetaVec(thetaMin);
        const Vector2D& thetaVecMax = trigonometricalLookupTable.thetaVec(thetaMax);

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

        // Check the extremum
        float rHitMinExtr = thetaVecMin.cross(pos2D);
        float rHitMaxExtr = thetaVecMax.cross(pos2D);
        if (rHitMinExtr * rHitMaxExtr < 0.) return checkExtremum(node, wireHit);

        // Not contained
        return false;
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       * @param node quadtree node
       * @param i theta index of the child
       * @param j rho index of the child
       * @return returns ranges of the (i;j) child
       */
      XYSpans createChild(QuadTree* node, unsigned int i, unsigned int j) const final
      {
        const int nodeLevel = node->getLevel();
        const int lastLevel = getLastLevel();
        float meanCurv = fabs(node->getYMean());

        // Expand bins for all nodes 7 levels before the last level (for lastLevel = 12 starting at 6)
        // but only in a curvature region higher than 0.005. Lower than that use always standard.
        bool standardBinning =
          m_standartBinning or (nodeLevel <= lastLevel - 7) or (meanCurv <= 0.005);

        if (standardBinning) {
          float r1 = node->getYBinBound(j);
          float r2 = node->getYBinBound(j + 1);
          long theta1 = node->getXBinBound(i);
          long theta2 = node->getXBinBound(i + 1);

          // Standard bin division
          return XYSpans({theta1, theta2}, {r1, r2});
        }

        // Non-standard binning
        // For level 6 to 7 only expand 1 / 4, for higher levels expand  1 / 8.
        // (assuming last level == 12)
        if (nodeLevel < lastLevel - 5) {
          float r1 = node->getYBinBound(j) - node->getYBinWidth(j) / 4.;
          float r2 = node->getYBinBound(j + 1) + node->getYBinWidth(j) / 4.;

          // long extension = pow(2, lastLevel - nodeLevel) / 4; is same as:
          long extension = pow(2, lastLevel - nodeLevel - 2);

          long theta1 = node->getXBinBound(i) - extension;
          if (theta1 < 0) theta1 = 0;

          long theta2 = node->getXBinBound(i + 1) + extension;
          if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta()) {
            theta2 = TrigonometricalLookupTable<>::Instance().getNBinsTheta() - 1;
          }

          return XYSpans({theta1, theta2}, {r1, r2});
        } else {
          float r1 = node->getYBinBound(j) - node->getYBinWidth(j) / 8.;
          float r2 = node->getYBinBound(j + 1) + node->getYBinWidth(j) / 8.;

          // long extension = pow(2, lastLevel - nodeLevel) / 8; is same as
          long extension = pow(2, lastLevel - nodeLevel - 3);

          long theta1 = node->getXBinBound(i) - extension;
          if (theta1 < 0) theta1 = 0;

          long theta2 = node->getXBinBound(i + 1) + extension;
          if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta()) {
            theta2 = TrigonometricalLookupTable<>::Instance().getNBinsTheta() - 1;
          }

          return XYSpans({theta1, theta2}, {r1, r2});
        }
      }

      /**
       * Fill m_quadTree vector with QuadTree instances (number of instances is 4^lvl).
       * @param lvl level to which QuadTree instances should be equal in sense of the rho-theta boundaries.
       */
      void seedQuadTree(int lvl)
      {
        bool twoSidedPhasespace(false);

        if ((m_quadTree->getYMin() * m_quadTree->getYMax()) < 0)twoSidedPhasespace = true;

        long nbins = pow(2, lvl);

        m_seededTree.reserve(nbins * nbins);

        XYSpans ranges({m_quadTree->getXMin(), m_quadTree->getXMax()},
        {m_quadTree->getYMin(), m_quadTree->getYMax()});


        const XSpan& xSpan = ranges.first;
        const YSpan& ySpan = ranges.second;

        long binSizeX = (xSpan[1] - xSpan[0]) / nbins;
        float binSizeY = (ySpan[1] - ySpan[0]) / nbins;

        std::vector<Item*> items = m_quadTree->getItems();

        for (long xIndex = 0; xIndex < nbins; xIndex++) {
          for (long yIndex = 0; yIndex < nbins; yIndex++) {

            long xMin = xIndex * binSizeX + xSpan[0];
            long xMax = (xIndex + 1) * binSizeX + xSpan[0];
            float yMin = yIndex * binSizeY + ySpan[0];
            float yMax = (yIndex + 1) * binSizeY + ySpan[0];

            m_seededTree.push_back(QuadTree({xMin, xMax}, {yMin, yMax}, lvl, nullptr));
            QuadTree& newQuadTree = m_seededTree.back();
            if ((newQuadTree.getYMin() < -0.02)  && (newQuadTree.getYMax() < -0.02)) continue;
            newQuadTree.reserveItems(m_quadTree->getNItems() * 2);

            for (Item* item : items) {
              if (item->isUsed()) continue;

              if (isInNode(&newQuadTree, item->getPointer())) {
                if (twoSidedPhasespace && (newQuadTree.getYMin() < 0.02)  && (newQuadTree.getYMax() < 0.02)) {
                  if (checkDerivative(&newQuadTree, item->getPointer())) {
                    newQuadTree.insertItem(item);
                  }
                } else {
                  newQuadTree.insertItem(item);
                }
              }
            }

            if (newQuadTree.getNItems() < 10) m_seededTree.pop_back();

          }
        }

        sortSeededTree();

      }

      /// Sort vector of seeded QuadTree instances by number of hits.
      void sortSeededTree()
      {
        std::sort(m_seededTree.begin(), m_seededTree.end(), [](QuadTree & quadTree1, QuadTree & quadTree2) { return quadTree1.getNItems() > quadTree2.getNItems();});
      }

      /**
       * Fill vector of QuadTree instances with hits.
       * @param lmdProcessor the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       * @param yLimit the threshold in the rho (curvature) variable
       */
      void fillSeededTree(CandidateProcessorLambda& lmdProcessor,
                          unsigned int nHitsThreshold, float yLimit)
      {
        sortSeededTree();
        for (QuadTree& tree : m_seededTree) {
          erase_remove_if(tree.getItems(), [&](Item * hit) { return hit->isUsed(); });
          fillGivenTree(&tree, lmdProcessor, nHitsThreshold, yLimit);
        }
      }

      /// Clear vector of QuadTree instances
      void clearSeededTree()
      {
        m_seededTree.clear();
        m_seededTree.shrink_to_fit();
      }

      /**
       * Create single QuadTree node and fill it with unused hits.
       * @param ranges ranges of the node
       * @return returns pointer to QuadTree instance
       */

      QuadTree* createSingleNode(const XYSpans& ranges)
      {
        std::vector<Item*> hitsVector = m_quadTree->getItems();

        const XSpan& xSpan = ranges.first;
        const YSpan& ySpan = ranges.second;
        QuadTree* quadTree = new QuadTree(xSpan, ySpan, 0, nullptr);

        std::vector<Item*>& quadtreeItems = quadTree->getItems();
//        quadtreeItems.reserve(hitsVector.size());
        for (Item* item : hitsVector) {
          if (item->isUsed()) continue;
          if (isInNode(quadTree, item->getPointer())) {
            quadtreeItems.push_back(item);
          }
        }

        return quadTree;
      }

      /// Draw QuadTree nodes
      void drawNode()
      {
        static int nevent(0);

        TCanvas* canv = new TCanvas("canv", "legendre transform", 0, 0, 1200, 600);
        canv->cd(1);
        TGraph* dummyGraph = new TGraph();
        dummyGraph->SetPoint(1, -3.1415, 0);
        dummyGraph->SetPoint(2, 3.1415, 0);
        dummyGraph->Draw("AP");
        dummyGraph->GetXaxis()->SetTitle("#theta");
        dummyGraph->GetYaxis()->SetTitle("#rho");
        dummyGraph->GetXaxis()->SetRangeUser(-3.1415, 3.1415);
        dummyGraph->GetYaxis()->SetRangeUser(0, 0.15);


        //    int nhits = 0;
        for (Item* hit : m_quadTree->getItems()) {
          TF1* funct1 = new TF1("funct", "2*[0]*cos(x)/((1-sin(x))*[1]) ", -3.1415, 3.1415);
          funct1->SetLineWidth(1);
          double r2 = (hit->getPointer()->getRefPos2D().norm() + hit->getPointer()->getRefDriftLength()) *
                      (hit->getPointer()->getRefPos2D().norm() - hit->getPointer()->getRefDriftLength());
          double d2 = hit->getPointer()->getRefDriftLength() * hit->getPointer()->getRefDriftLength();
          double x = hit->getPointer()->getRefPos2D().x();

          funct1->SetParameters(x, r2 - d2);
          funct1->Draw("CSAME");

        }
        canv->Print(Form("legendreHits_%i.root", nevent));
        canv->Print(Form("legendreHits_%i.eps", nevent));
        canv->Print(Form("legendreHits_%i.png", nevent));


        nevent++;
      }

      /**
       * Check derivative of the sinogram.
       * @param node QuadTree node
       * @param hit pointer to the hit to check
       * @return returns true in cases:
       * @return    - positive derivative and no extremum in the node's ranges or
       * @return    - extremum located in the node's ranges
       * @return returns false in other cases (namely negative derivative
       *
       */
      bool checkDerivative(QuadTree* node, const CDCWireHit* wireHit) const
      {
        const Vector2D& pos2D = wireHit->getRefPos2D();

        long thetaMin = node->getXMin();
        long thetaMax = node->getXMax();

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();
        const Vector2D& thetaVecMin = trigonometricalLookupTable.thetaVec(thetaMin);
        const Vector2D& thetaVecMax = trigonometricalLookupTable.thetaVec(thetaMax);

        float rMinD = thetaVecMin.cross(pos2D);
        float rMaxD = thetaVecMax.cross(pos2D);

        // Does not really make sense...
        if ((rMinD > 0) && (rMaxD * rMinD >= 0)) return true;
        if ((rMaxD * rMinD < 0)) return true;
        return false;
      }

      /**
       * Checks whether extremum point is located whithin QuadTree node's ranges
       * @param node QuadTree node
       * @param hit hit to check
       * @return true or false
       */
      bool checkExtremum(QuadTree* node, const CDCWireHit* wireHit) const
      {
        const double& l = wireHit->getRefDriftLength();
        const Vector2D& pos2D = wireHit->getRefPos2D();
        double r2 = square(wireHit->getRefCylindricalR()) - l * l;

        // get left and right borders of the node
        long thetaMin = node->getXMin();
        long thetaMax = node->getXMax();

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();
        const Vector2D& thetaVecMin = trigonometricalLookupTable.thetaVec(thetaMin);
        const Vector2D& thetaVecMax = trigonometricalLookupTable.thetaVec(thetaMax);

        if (not pos2D.isBetween(thetaVecMin, thetaVecMax)) return false;

        // compute sinograms at the position
        double r = wireHit->getRefCylindricalR();
        float rRight = r - l;
        float rLeft = r + l;

        // get top and bottom borders of the node
        float rMin = node->getYMin() * r2 / 2;
        float rMax = node->getYMax() * r2 / 2;

        bool crossesRight = (rMin - rRight) * (rMax - rRight) < 0;
        bool crossesLeft = (rMin - rLeft) * (rMax - rLeft) < 0;
        return crossesRight or crossesLeft;
      }
    };

  }
}
