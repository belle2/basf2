/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/BoxDivisionHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

#include <TGraph.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TAxis.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Convenience class for the typical usage-case: A box divisioned hough tree with maximum and minimum values in both directions.
    /// This time in 3D
    template<class AHitPtr, class AInBoxAlgorithm, size_t divisionX, size_t divisionY, size_t divisionZ>
    class SimpleBoxDivisionHoughTree3D : public
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
      SimpleBoxDivisionHoughTree3D(float maximumX,
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
        Super::template constructArray<1>(-getMaximumY(), getMaximumY(), getOverlapY());
        Super::template constructArray<2>(-getMaximumZ(), getMaximumZ(), getOverlapZ());

        Super::initialize();
      }

      /// Find only the leaf with the highest weight (~= number of items)
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

      /// Write debug information into a ROOT file; not implemented
      void writeDebugInfoToFile(const std::string& filename __attribute__((unused)))
      {
        //do nothing;
      }

      /**
       * Draws found hits and node boundaries
       * FIXME this is a copy-paste from DebugableSimpleBoxDivisionHoughTree
       * It should be possible to unify it with this tree, but not sure of does worth it
       */
      void drawDebugPlot(const std::vector<CDCRecoHit3D>& allHits,
                         const std::vector<CDCRecoHit3D>& foundHits,
                         const typename AInBoxAlgorithm::HoughBox& node)
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

        const double centerX = (AInBoxAlgorithm::BoxAlgorithm::centerX(node));
        const double deltaX = (AInBoxAlgorithm::BoxAlgorithm::deltaX(node));
        const double centerY = (AInBoxAlgorithm::BoxAlgorithm::centerY(node));
        const double centerZ = (AInBoxAlgorithm::BoxAlgorithm::centerZ(node));

        TF1* candidateL = new TF1("candL", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateH = new TF1("candH", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);
        TF1* candidateMean = new TF1("candMean", AInBoxAlgorithm::BoxAlgorithm::debugLine(), 0, 120);

        candidateL->SetParameters(centerX - deltaX, centerY, centerZ - 100.0 * deltaX);
        candidateH->SetParameters(centerX + deltaX, centerY, centerZ + 100.0 * deltaX);
        candidateMean->SetParameters(centerX, centerY, centerZ);

        candidateL->SetLineColor(9);
        candidateH->SetLineColor(41);
        candidateMean->SetLineColor(2);

        candidateL->Draw("same");
        candidateH->Draw("same");
        candidateMean->Draw("same");
        canv.SaveAs(Form("CDCRLHits_%i.png", nevent));
        nevent++;
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
