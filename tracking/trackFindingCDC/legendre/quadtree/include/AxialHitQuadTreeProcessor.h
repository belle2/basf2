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
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include "tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h"

#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class QuadTreeHitWrapper;

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessorTemplate<unsigned long, float, QuadTreeHitWrapper, 2, 2> {

    public:

      AxialHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges, std::function< double(double) >& lmdFunctLevel,
                                bool standartBinning = false) :
        QuadTreeProcessorTemplate(lastLevel, ranges), m_standartBinning(standartBinning), m_lmdFunctLevel(lmdFunctLevel) { }

    private:

      bool m_standartBinning;

      std::function< double(double) >& m_lmdFunctLevel;

      inline bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

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
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      inline bool insertItemInNode(QuadTree* node, QuadTreeHitWrapper* hit, unsigned int /*t_index*/,
                                   unsigned int /*r_index*/) const override final
      {
        float dist_1[2][2];
        float dist_2[2][2];
        /*
        double r2 = (hit->getCDCWireHit()->getRefPos2D().norm() + hit->getCDCWireHit()->getRefDriftLength()) *
                    (hit->getCDCWireHit()->getRefPos2D().norm() - hit->getCDCWireHit()->getRefDriftLength());
        double d2 = hit->getCDCWireHit()->getRefDriftLength()*hit->getCDCWireHit()->getRefDriftLength();

        int thetaMin = node->getXMin();
        int thetaMax = node->getXMax();
        float rMin = node->getYMin();
        float rMax = node->getYMax();

        float cosThetaMin = TrigonometricalLookupTable::Instance().cosTheta(abs(thetaMin));
        float sinThetaMin = TrigonometricalLookupTable::Instance().sinTheta(abs(thetaMin));
        float cosThetaMax = TrigonometricalLookupTable::Instance().cosTheta(abs(thetaMax));
        float sinThetaMax = TrigonometricalLookupTable::Instance().sinTheta(abs(thetaMax));


        //left border of the node:
        float kappaHitMin = (cosThetaMin/(1-sinThetaMin)) * (2.*hit->getCDCWireHit()->getRefPos2D().x()/(r2-d2));
        //right border of the node:
        float kappaHitMax = (cosThetaMax/(1-sinThetaMax)) * (2.*hit->getCDCWireHit()->getRefPos2D().x()/(r2-d2));

        float rHitMin1 = kappaHitMin - hit->getConformalDriftLength();
        float rHitMin2 = kappaHitMin + hit->getConformalDriftLength();
        float rHitMax1 = kappaHitMax - hit->getConformalDriftLength();
        float rHitMax2 = kappaHitMax + hit->getConformalDriftLength();

        dist_1[0][0] = rMin - rHitMin1;
        dist_1[0][1] = rMin - rHitMax1;
        dist_1[1][0] = rMax - rHitMin1;
        dist_1[1][1] = rMax - rHitMax1;

        dist_2[0][0] = rMin - rHitMin2;
        dist_2[0][1] = rMin - rHitMax2;
        dist_2[1][0] = rMax - rHitMin2;
        dist_2[1][1] = rMax - rHitMax2;


        */
        unsigned long thetaMin = node->getXMin();
        unsigned long thetaMax = node->getXMax();
        float rMin = node->getYMin();
        float rMax = node->getYMax();
        float cosThetaMin = TrigonometricalLookupTable::Instance().cosTheta(abs(thetaMin));
        float sinThetaMin = TrigonometricalLookupTable::Instance().sinTheta(abs(thetaMin));
        float cosThetaMax = TrigonometricalLookupTable::Instance().cosTheta(abs(thetaMax));
        float sinThetaMax = TrigonometricalLookupTable::Instance().sinTheta(abs(thetaMax));

        float rHitMin = hit->getConformalX() * cosThetaMin + hit->getConformalY() * sinThetaMin;
        float rHitMax = hit->getConformalX() * cosThetaMax + hit->getConformalY() * sinThetaMax;

        float rHitMin1 = rHitMin - hit->getConformalDriftLength();
        float rHitMin2 = rHitMin + hit->getConformalDriftLength();
        float rHitMax1 = rHitMax - hit->getConformalDriftLength();
        float rHitMax2 = rHitMax + hit->getConformalDriftLength();

        dist_1[0][0] = rMin - rHitMin1;
        dist_1[0][1] = rMin - rHitMax1;
        dist_1[1][0] = rMax - rHitMin1;
        dist_1[1][1] = rMax - rHitMax1;

        dist_2[0][0] = rMin - rHitMin2;
        dist_2[0][1] = rMin - rHitMax2;
        dist_2[1][0] = rMax - rHitMin2;
        dist_2[1][1] = rMax - rHitMax2;


        bool valueToReturn(false);

        if (! sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1])) valueToReturn = true;
        else if (! sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1])) valueToReturn = true;
        else {
          bool extremumInside(false);

          float rHitMinExtr = -1.*hit->getConformalX() * sinThetaMin + hit->getConformalY() * cosThetaMin;
          float rHitMaxExtr = -1.*hit->getConformalX() * sinThetaMax + hit->getConformalY() * cosThetaMax;

          if (rHitMinExtr * rHitMaxExtr < 0.) valueToReturn = checkExtremum(node, hit);
        }

        return valueToReturn;
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       */
      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override final
      {
        if (not m_standartBinning) {
          if ((node->getLevel() > (getLastLevel() - 7)) && (fabs(node->getYMean()) > 0.005)) {
            if (node->getLevel() < (getLastLevel() - 5)) {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 4.;
              unsigned long theta1 = node->getXBin(i) - std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 4.);
              unsigned long theta2 = node->getXBin(i + 1) + std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 4.);

              if (theta1 < 0)
                theta1 = node->getXBin(i);
              if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                theta2 = node->getXBin(i + 1);

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            } else {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 8.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 8.;
              unsigned long theta1 = node->getXBin(i) - std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 8.);
              unsigned long theta2 = node->getXBin(i + 1) + std::abs(pow(2, getLastLevel() + 0 - node->getLevel()) / 8.);

              if (theta1 < 0)
                theta1 = node->getXBin(i);
              if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                theta2 = node->getXBin(i + 1);

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            }
          } else {
            return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
          }
        } else {
          return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
        }
      }


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
          if (insertItemInNode(quadTree, item->getPointer(), 0, 0))
            quadtreeItemsVector.push_back(item);
        }

        return quadTree;
      }


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
          double r2 = (hit->getPointer()->getCDCWireHit()->getRefPos2D().norm() + hit->getPointer()->getCDCWireHit()->getRefDriftLength()) *
                      (hit->getPointer()->getCDCWireHit()->getRefPos2D().norm() - hit->getPointer()->getCDCWireHit()->getRefDriftLength());
          double d2 = hit->getPointer()->getCDCWireHit()->getRefDriftLength() * hit->getPointer()->getCDCWireHit()->getRefDriftLength();
          double x = hit->getPointer()->getCDCWireHit()->getRefPos2D().x();

          funct1->SetParameters(x, r2 - d2);
          funct1->Draw("CSAME");

        }
        canv->Print(Form("legendreHits_%i.root", nevent));
        canv->Print(Form("legendreHits_%i.eps", nevent));
        canv->Print(Form("legendreHits_%i.png", nevent));


        nevent++;
      }

      bool checkExtremum(QuadTree* node, QuadTreeHitWrapper* hit) const
      {

        double thetaExtremum = atan2(hit->getConformalY(), hit->getConformalX());

        double pi = boost::math::constants::pi<double>();

        //        if (thetaExtremum > pi) thetaExtremum -= pi;
        //        if (thetaExtremum < 0.) thetaExtremum += pi;

        unsigned long thetaExtremumLookup = (thetaExtremum + pi) * TrigonometricalLookupTable::Instance().getNBinsTheta() / (2.*pi) ;

        if ((thetaExtremumLookup > node->getXMax()) || (thetaExtremumLookup < node->getXMin())) return false;

        double rPrim = hit->getConformalX() * TrigonometricalLookupTable::Instance().cosTheta(thetaExtremumLookup)
                       + hit->getConformalY() * TrigonometricalLookupTable::Instance().sinTheta(thetaExtremumLookup);

        if ((rPrim > node->getYMin()) && (rPrim < node->getYMax())) return true;


        return false;
      }

    };

  }
}
