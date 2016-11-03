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
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCConformalHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/BasePrecisionFunction.h>


#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>

#include <array>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCConformalHit;

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessorTemplate<unsigned long, float, CDCConformalHit, 2, 2> {

    public:

      /// Constructor
      AxialHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges,
                                BasePrecisionFunction::PrecisionFunction& lmdFunctLevel,
                                bool standartBinning = false) :
        QuadTreeProcessorTemplate(lastLevel, ranges), m_standartBinning(standartBinning), m_lmdFunctLevel(lmdFunctLevel) { }

    private:

      /**
       *  Sets whether standard splitting of bins will be used
       *   - in case of standard binning each bin will be splitted into 4 equal bins
       *   - in case of non-standard binning boundaries of each child will be extended (see AxialHitQuadTreeProcessor::createChildWithParent())
       */
      bool m_standartBinning;

      /// Lambda which holds resolution function for the quadtree
      BasePrecisionFunction::PrecisionFunction& m_lmdFunctLevel;

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
      bool checkIfLastLevel(QuadTree* node) override final
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
      bool insertItemInNode(QuadTree* node, CDCConformalHit* hit, unsigned int /*t_index*/,
                            unsigned int /*r_index*/) const override final
      {
        using Quadlet = std::array< std::array<float, 2>, 2>;
        Quadlet dist_1;
        Quadlet dist_2;

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();

        // get left and right borders of the node
        unsigned long thetaMin = node->getXMin();
        unsigned long thetaMax = node->getXMax();

        // get top and bottom borders of the node
        float rMin = node->getYMin();
        float rMax = node->getYMax();

        Vector2D thetaVecMin = trigonometricalLookupTable.thetaVec(thetaMin);
        Vector2D thetaVecMax = trigonometricalLookupTable.thetaVec(thetaMax);

        float rHitMin = thetaVecMin.dot(hit->getConformalPos2D());
        float rHitMax = thetaVecMax.dot(hit->getConformalPos2D());

        // compute sinograms at the left and right borders of the node
        float rHitMin1 = rHitMin - hit->getConformalDriftLength();
        float rHitMin2 = rHitMin + hit->getConformalDriftLength();
        float rHitMax1 = rHitMax - hit->getConformalDriftLength();
        float rHitMax2 = rHitMax + hit->getConformalDriftLength();

        //compute distance from the sinograms to bottom and top borders of the node

        // this has some explicit cppcheck suppressions to silence false-positives about
        // array bounds
        dist_1[0][0] = rMin - rHitMin1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[0][1] = rMin - rHitMax1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[1][0] = rMax - rHitMin1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[1][1] = rMax - rHitMax1;

        dist_2[0][0] = rMin - rHitMin2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[0][1] = rMin - rHitMax2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[1][0] = rMax - rHitMin2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[1][1] = rMax - rHitMax2;

        bool valueToReturn(false);

        // compare distances from sinograms to the node -- basing on this information we check for the affiliation of the hit to the node
        // cppcheck-suppress arrayIndexOutOfBounds
        if (! sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1])) {
          valueToReturn = true;
        }
        // cppcheck-suppress arrayIndexOutOfBounds
        else if (! sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1])) {
          valueToReturn = true;
        } else {

          float rHitMinExtr = thetaVecMin.cross(hit->getConformalPos2D());
          float rHitMaxExtr = thetaVecMax.cross(hit->getConformalPos2D());
          if (rHitMinExtr * rHitMaxExtr < 0.) valueToReturn = checkExtremum(node, hit);
        }

        return valueToReturn;
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       * @param node quadtree node
       * @param i theta index of the child
       * @param j rho index of the child
       * @return returns ranges of the (i;j) child
       */
      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override final
      {
        if (not m_standartBinning) {
          if ((node->getLevel() > (getLastLevel() - 7)) && (fabs(node->getYMean()) > 0.005)) {
            if (node->getLevel() < (getLastLevel() - 5)) {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 4.;
              unsigned long extension = static_cast<unsigned long>(pow(2, getLastLevel() - node->getLevel()) / 4);

              unsigned long theta1 = node->getXBin(i);
              if (theta1 <= extension) {
                theta1 = 0;
              } else {
                theta1 -= extension;
              }

              unsigned long theta2 = node->getXBin(i + 1) + extension;
              if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta()) {
                theta2 = node->getXBin(i + 1);
              }

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            } else {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 8.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 8.;
              unsigned long extension = static_cast<unsigned long>(pow(2, getLastLevel() - node->getLevel()) / 8);

              unsigned long theta1 = node->getXBin(i);
              if (theta1 <= extension) {
                theta1 = 0;
              } else {
                theta1 -= extension;
              }

              unsigned long theta2 = node->getXBin(i + 1) + extension;
              if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta()) {
                theta2 = node->getXBin(i + 1);
              }

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            }
          } else {
            return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
          }
        } else {
          return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
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

        unsigned long nbins = pow(2, lvl);

        m_seededTree.reserve(nbins * nbins);

        ChildRanges ranges = std::make_pair(rangeX(m_quadTree->getXMin(), m_quadTree->getXMax()),
                                            rangeY(m_quadTree->getYMin(), m_quadTree->getYMax()));


        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;

        unsigned long binSizeX = (x.second - x.first) / nbins;
        float binSizeY = (y.second - y.first) / nbins;

        std::vector<ItemType*> items = m_quadTree->getItemsVector();

        for (unsigned long xIndex = 0; xIndex < nbins; xIndex++) {
          for (unsigned long yIndex = 0; yIndex < nbins; yIndex++) {
            unsigned long xMin = xIndex * binSizeX + x.first;
            unsigned long xMax = (xIndex + 1) * binSizeX + x.first;
            float yMin = yIndex * binSizeY + y.first;
            float yMax = (yIndex + 1) * binSizeY + y.first;

            m_seededTree.emplace_back(xMin, xMax, yMin, yMax, lvl, nullptr);
            QuadTree& newQuadTree = m_seededTree.back();
            if ((newQuadTree.getYMin() < -0.02)  && (newQuadTree.getYMax() < -0.02)) continue;
            newQuadTree.reserveHitsVector(m_quadTree->getNItems() * 2);

            for (ItemType* item : items) {
              if (item->isUsed()) continue;

              if (insertItemInNode(&newQuadTree, item->getPointer(), 0, 0)) {
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
       * @param rThreshold the threshold in the rho (curvature) variable
       */
      void fillSeededTree(CandidateProcessorLambda& lmdProcessor,
                          unsigned int nHitsThreshold, float rThreshold)
      {
        sortSeededTree();
//        B2INFO("Size of the tree : " << m_seededTree.size());

        for (QuadTree& tree : m_seededTree) {
//          B2INFO("NItems in the node: " << tree.getNItems());
//          fillGivenTreeWithSegments(&tree, lmdProcessor, nHitsThreshold, rThreshold, true);
          tree.cleanUpItems(*this);
          fillGivenTree(&tree, lmdProcessor, nHitsThreshold, rThreshold, true);
        }

      }

      /// Clear vector of QuadTree instances
      void clearSeededTree()
      {
        m_seededTree.clear();
        m_seededTree.resize(0);
      }

      /**
       * Create single QuadTree node and fill it with unused hits.
       * @param ranges ranges of the node
       * @return returns pointer to QuadTree instance
       */

      QuadTree* createSingleNode(const ChildRanges& ranges)
      {
        std::vector<ItemType*> hitsVector = m_quadTree->getItemsVector();

        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;
        QuadTree* quadTree = new QuadTree(x.first, x.second, y.first, y.second, 0, nullptr);

        std::vector<ItemType*>& quadtreeItemsVector = quadTree->getItemsVector();
//        quadtreeItemsVector.reserve(hitsVector.size());
        for (ItemType* item : hitsVector) {
          if (item->isUsed()) continue;
          if (insertItemInNode(quadTree, item->getPointer(), 0, 0)) {
            quadtreeItemsVector.push_back(item);
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
        for (ItemType* hit : m_quadTree->getItemsVector()) {
          TF1* funct1 = new TF1("funct", "2*[0]*cos(x)/((1-sin(x))*[1]) ", -3.1415, 3.1415);
          funct1->SetLineWidth(1);
          double r2 = (hit->getPointer()->getWireHit()->getRefPos2D().norm() + hit->getPointer()->getWireHit()->getRefDriftLength()) *
                      (hit->getPointer()->getWireHit()->getRefPos2D().norm() - hit->getPointer()->getWireHit()->getRefDriftLength());
          double d2 = hit->getPointer()->getWireHit()->getRefDriftLength() * hit->getPointer()->getWireHit()->getRefDriftLength();
          double x = hit->getPointer()->getWireHit()->getRefPos2D().x();

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
      bool checkDerivative(QuadTree* node, CDCConformalHit* hit) const
      {
        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();


        float rMinD = trigonometricalLookupTable.thetaVec(node->getXMin()).cross(hit->getConformalPos2D());
        float rMaxD = trigonometricalLookupTable.thetaVec(node->getXMax()).cross(hit->getConformalPos2D());

        // float rMean = node->getYMean();
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
      bool checkExtremum(QuadTree* node, CDCConformalHit* hit) const
      {

        double thetaExtremum = hit->getConformalPos2D().phi();

        double pi = boost::math::constants::pi<double>();

        //        if (thetaExtremum > pi) thetaExtremum -= pi;
        //        if (thetaExtremum < 0.) thetaExtremum += pi;

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();

        unsigned long thetaExtremumLookup = (thetaExtremum + pi) * trigonometricalLookupTable.getNBinsTheta() / (2.*pi) ;

        if ((thetaExtremumLookup > node->getXMax()) || (thetaExtremumLookup < node->getXMin())) return false;

        Vector2D thetaVec = trigonometricalLookupTable.thetaVec(thetaExtremumLookup);
        double rD = thetaVec.dot(hit->getConformalPos2D());
        if ((rD > node->getYMin()) && (rD < node->getYMax())) return true;

        return false;
      }

    };

  }
}
