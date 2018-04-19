/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/collectors/matchers/StereoHitTrackQuadTreeMatcher.h>

#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>
#include <tracking/trackFindingCDC/hough/quadratic/HitQuadLegendre.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <TGraph.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

template <class AQuadTree>
void StereoHitTrackQuadTreeMatcher<AQuadTree>::exposeParameters(ModuleParamList* moduleParamList,
    const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "level"), m_param_quadTreeLevel,
                                "The number of levels for the quad tree search.",
                                m_param_quadTreeLevel);

  moduleParamList->addParameter(prefixed(prefix, "minimumNumberOfHits"), m_param_minimumNumberOfHits,
                                "The minimum number of hits in a quad tree bin to be called as result.",
                                m_param_minimumNumberOfHits);

  moduleParamList->addParameter(prefixed(prefix, "writeDebugInformation"), m_param_writeDebugInformation,
                                "Set to true to output debug information.",
                                m_param_writeDebugInformation);

  moduleParamList->addParameter(prefixed(prefix, "checkForB2BTracks"),
                                m_param_checkForB2BTracks,
                                "Set to false to skip the check for back-2-back tracks "
                                "(good for cosmics).",
                                m_param_checkForB2BTracks);

  moduleParamList->addParameter(prefixed(prefix, "checkForInWireBoundsFactor"),
                                m_param_checkForInWireBoundsFactor,
                                "Used to scale the CDC before checking "
                                "whether hits are in the CDC z bounds.",
                                m_param_checkForInWireBoundsFactor);
}


template <class AQuadTree>
void StereoHitTrackQuadTreeMatcher<AQuadTree>::initialize()
{
  Super::initialize();

  m_quadTreeInstance.setMaxLevel(m_param_quadTreeLevel);
  m_quadTreeInstance.initialize();
}


template <class AQuadTree>
void StereoHitTrackQuadTreeMatcher<AQuadTree>::terminate()
{
  Super::terminate();

  m_quadTreeInstance.raze();
}

template <class AQuadTree>
void StereoHitTrackQuadTreeMatcher<AQuadTree>::match(CDCTrack& track, const std::vector<CDCRLWireHit>& rlWireHits,
                                                     std::vector<Super::WeightedRelationItem>& relationsForCollector)
{
  // TODO: Extract this into smaller steps
  // TODO: Split the filtering from the rest.
  // This means this method would output WeightedRelations based on the quad tree decision and a second filter step
  // can be applied on these weighted relations

  // Reconstruct the hits to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const bool isCurler = trajectory2D.isCurler();

  using CDCRecoHitWithRLPointer = std::pair<CDCRecoHit3D, const CDCRLWireHit*>;
  std::vector<CDCRecoHitWithRLPointer> recoHits;
  recoHits.reserve(rlWireHits.size() + track.size());

  /*
   * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
   * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
   */
  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag()) continue;

    const CDCWire& wire = rlWireHit.getWire();
    const WireLine& wireLine = wire.getWireLine();
    double signedDriftLength = rlWireHit.getSignedRefDriftLength();
    for (const Vector3D& recoPos3D : trajectory2D.reconstructBoth3D(wireLine, signedDriftLength)) {
      // Skip hits out of CDC
      if (not wire.isInCellZBounds(recoPos3D, m_param_checkForInWireBoundsFactor)) {
        continue;
      }

      // If the track is a curler, shift all perpS values to positive ones.
      // Else do not use this hit if m_param_checkForB2BTracks is enabled.
      double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
//       if (perpS < 0) {
//         if (isCurler) {
//           perpS += trajectory2D.getArcLength2DPeriod();
//         } else if (m_param_checkForB2BTracks) {
//           continue;
//         }
//       }
      recoHits.emplace_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS), &rlWireHit);
    }
  }

  // Also add already found stereo hits of the track
  for (const CDCRecoHit3D& recoHit : track) {
    if (not recoHit.isAxial()) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      const CDCRLWireHit& rlWireHit = recoHit.getRLWireHit();
      recoHits.emplace_back(recoHit, &rlWireHit);
    }
  }

  // Do the tree finding
  m_quadTreeInstance.seed(recoHits);

  if (m_param_writeDebugInformation) {
    writeDebugInformation();
  }

  const auto& foundStereoHitsWithNode = m_quadTreeInstance.findSingleBest(m_param_minimumNumberOfHits);
  m_quadTreeInstance.fell();

  if (foundStereoHitsWithNode.size() != 1) {
    return;
  }

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  const auto& node = foundStereoHitsWithNode[0].first;

  //------- DEBUG STUFF FIXME DELETE OR MAKE A NICE FUNCTION SOMEWHERE ----------------------
  {
    TGraph* allHits = new TGraph();
    allHits->SetLineWidth(2);
    allHits->SetLineColor(9);

    for (const CDCRecoHitWithRLPointer recoHitWithRL : recoHits) {
      const CDCRecoHit3D& recoHit3D = recoHitWithRL.first;
      const Vector3D& recoPos3D = recoHit3D.getRecoPos3D();
      const double R = std::sqrt(recoPos3D.x() * recoPos3D.x() + recoPos3D.y() * recoPos3D.y());
      const double Z = recoPos3D.z();
      allHits->SetPoint(allHits->GetN(), R, Z);
    }

    static int nevent(0);
    TCanvas canv("trackCanvas", "CDC stereo hits in an event", 0, 0, 800, 600);
    canv.cd();
    allHits->Draw("APL*");
    allHits->GetXaxis()->SetLimits(0, 120);
    allHits->GetYaxis()->SetRangeUser(-180, 180);

    TGraph* foundHits = new TGraph();
    foundHits->SetMarkerStyle(2);
    foundHits->SetMarkerColor(2);

    for (const auto& recoHit : foundStereoHits) {
      const Vector3D& recoPos3D = recoHit.first.getRecoPos3D();
      const double R = std::sqrt(recoPos3D.x() * recoPos3D.x() + recoPos3D.y() * recoPos3D.y());
      const double Z = recoPos3D.z();
      foundHits->SetPoint(foundHits->GetN(), R, Z);
    }
    allHits->Draw("same");

    const double xMean = (node.getLowerX() + node.getUpperX()) / 2.0; //Z0 or Z1
    const double yMean = (node.getLowerY() + node.getUpperY()) / 2.0; //tanLambda or Z2
    const double xLow = node.getLowerX();
    const double yLow = node.getLowerY();
    const double xHigh = node.getUpperX();
    const double yHigh = node.getUpperY();

    TF1* candidateLL = new TF1("candLL", "([0] + 4*[1])*x - [1] / 25 * x * x", 0, 120);
    TF1* candidateLH = new TF1("candLH", "([0] + 4*[1])*x - [1] / 25 * x * x", 0, 120);
    TF1* candidateHL = new TF1("candHL", "([0] + 4*[1])*x - [1] / 25 * x * x", 0, 120);
    TF1* candidateHH = new TF1("candHH", "([0] + 4*[1])*x - [1] / 25 * x * x", 0, 120);
    TF1* candidateMean = new TF1("candMean", "([0] + 4*[1])*x - [1] / 25 * x * x", 0, 120);

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
  //------- DEBUG STUFF FIXME DELETE OR MAKE A NICE FUNCTION SOMEWHERE ----------------------

  // Remove all assigned hits, which where already found before (and do not need to be added again)
  const auto& isAssignedHit = [](const CDCRecoHitWithRLPointer & recoHitWithRLPointer) {
    const CDCRecoHit3D& recoHit3D = recoHitWithRLPointer.first;
    const auto& automatonCell = recoHit3D.getWireHit().getAutomatonCell();
    return automatonCell.hasAssignedFlag();
  };

  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
                                       foundStereoHits.end(),
                                       isAssignedHit),
                        foundStereoHits.end());

  // Sort the found stereo hits by same CDCHit and smaller distance to the node
  // FIXME there should be a way to call the right function depending on the templated class
  // i.e. .getLowerZ0() and .getLowerTanLambda() or .getLowerZ1() and .getLowerZ2()
  const double xMean = (node.getLowerX() + node.getUpperX()) / 2.0; //Z0 or Z1
  const double yMean = (node.getLowerY() + node.getUpperY()) / 2.0; //tanLambda or Z2
  auto sortByHitAndNodeCenterDistance = [xMean, yMean](const CDCRecoHitWithRLPointer & lhs,
  const CDCRecoHitWithRLPointer & rhs) {

    const CDCRecoHit3D& rhsRecoHit = rhs.first;
    const CDCRecoHit3D& lhsRecoHit = lhs.first;

    const CDCWireHit& rhsWireHit = rhsRecoHit.getWireHit();
    const CDCWireHit& lhsWireHit = lhsRecoHit.getWireHit();

    if (lhsWireHit < rhsWireHit) {
      return true;
    } else if (rhsWireHit < lhsWireHit)  {
      return false;
    } else {
      const double lhsZ = lhsRecoHit.getRecoZ();
      const double rhsZ = rhsRecoHit.getRecoZ();

//       const double lhsS = lhsRecoHit.getArcLength2D();
//       const double rhsS = rhsRecoHit.getArcLength2D();
      const double lhsS = lhsRecoHit.getRecoPos2D().norm();
      const double rhsS = rhsRecoHit.getRecoPos2D().norm();

      double lhsZDistance;
      double rhsZDistance;

      if (AQuadTree::m_lookingForQuadraticTracks) {
//         lhsZDistance = lhsS * xMean + lhsS * lhsS * yMean - lhsZ;
//         rhsZDistance = rhsS * xMean + rhsS * rhsS * yMean - rhsZ;
        lhsZDistance = (xMean + 4 * yMean) * lhsS - yMean / 25 * lhsS * lhsS - lhsZ;
        rhsZDistance = (xMean + 4 * yMean) * rhsS - yMean / 25 * rhsS * rhsS - rhsZ;
      } else {
        lhsZDistance = lhsS * yMean + xMean - lhsZ;
        rhsZDistance = rhsS * yMean + xMean - rhsZ;
      }

      return lhsZDistance < rhsZDistance;
    }
  };

  const auto& sameHitComparer = [](const CDCRecoHitWithRLPointer & lhs,
  const CDCRecoHitWithRLPointer & rhs) {
    const CDCRecoHit3D& rhsRecoHit = rhs.first;
    const CDCRecoHit3D& lhsRecoHit = lhs.first;

    return lhsRecoHit.getWireHit() == rhsRecoHit.getWireHit();
  };

  std::sort(foundStereoHits.begin(),
            foundStereoHits.end(),
            sortByHitAndNodeCenterDistance);

  // If the same hit is added as right and left hypothesis, do only use the one with the smaller distance to the node.
  foundStereoHits.erase(std::unique(foundStereoHits.begin(),
                                    foundStereoHits.end(),
                                    sameHitComparer),
                        foundStereoHits.end());

  // Add the found stereo hits to the relation vector. In the moment, all hits get the same weight (may change later).
  for (const CDCRecoHitWithRLPointer& recoHitWithRLPointer : foundStereoHits) {
    const CDCRLWireHit* rlWireHit = recoHitWithRLPointer.second;
    relationsForCollector.emplace_back(&track, foundStereoHits.size(), rlWireHit);
  }
}

template <class AQuadTree>
void StereoHitTrackQuadTreeMatcher<AQuadTree>::writeDebugInformation()
{
  std::string outputFileName = "quadTreeContent_call_" + std::to_string(m_numberOfPassedDebugCalls) + ".root";
  m_quadTreeInstance.writeDebugInfoToFile(outputFileName);

  m_numberOfPassedDebugCalls++;
}

template class Belle2::TrackFindingCDC::StereoHitTrackQuadTreeMatcher<HitZ0TanLambdaLegendre>;
template class Belle2::TrackFindingCDC::StereoHitTrackQuadTreeMatcher<HitQuadLegendre>;
