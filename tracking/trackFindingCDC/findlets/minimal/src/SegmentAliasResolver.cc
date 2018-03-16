/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentAliasResolver.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  void swapBetterChi2(CDCSegment2D& segment, CDCSegment2D& aliasSegment)
  {
    const CDCTrajectory2D& aliasTrajectory2D = aliasSegment.getTrajectory2D();
    if (aliasTrajectory2D.isFitted() and
        aliasTrajectory2D.getChi2() < segment.getTrajectory2D().getChi2()) {
      aliasSegment.setAliasScore(NAN);
      std::swap(segment, aliasSegment);
    }
  }
}

std::string SegmentAliasResolver::getDescription()
{
  return "Resolves the rl aliasing of segments in various scenarios";
}

void SegmentAliasResolver::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "investigateAlias"),
                                m_param_investigateAlias,
                                "Which alias resolutions should be applied. "
                                "Options are trailing_hit, last_hit, full.",
                                m_param_investigateAlias);

  moduleParamList->addParameter(prefixed(prefix, "reestimateDriftLength"),
                                m_param_reestimateDriftLength,
                                "Switch to reestimate the drift length in the alias resolver",
                                m_param_reestimateDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "reestimatePositions"),
                                m_param_reestimatePositions,
                                "Switch to reestimate the reconstructed positions in the alias resolver",
                                m_param_reestimatePositions);
}

void SegmentAliasResolver::initialize()
{
  Super::initialize();
  for (const std::string& investigatedAlias : m_param_investigateAlias) {
    if (investigatedAlias == "full") {
      m_fullAlias = true;
    } else if (investigatedAlias == "cross") {
      m_crossAliases = true;
    } else if (investigatedAlias == "borders") {
      m_borderAliases = true;
    } else if (investigatedAlias == "middle") {
      m_middleAliases = true;
    } else {
      B2ERROR("Unknown alias to investigate " << investigatedAlias);
    }
  }
}

void SegmentAliasResolver::apply(std::vector<CDCSegment2D>& outputSegments)
{
  if (m_fullAlias) {
    for (CDCSegment2D& segment : outputSegments) {
      if (segment->hasAliasFlag()) continue; // A full alias has been found in the facet ca.
      int nRLSwitches = segment.getNRLSwitches();
      // Sufficiently right left constrained that the alias is already fixed.
      bool aliasStable = nRLSwitches > 2;
      if (aliasStable) continue;

      CDCSegment2D aliasSegment = segment.getAlias();
      refit(aliasSegment, true);
      swapBetterChi2(segment, aliasSegment);
    } // end alias loop
  }

  // Detect whether the segment has a waist and flip the rest of the segment to right
  if (m_crossAliases) {
    for (CDCSegment2D& segment : outputSegments) {
      int nRLSwitches = segment.getNRLSwitches();
      // Sufficiently right left constrained that the alias is already fixed.
      bool aliasStable = nRLSwitches > 2;
      if (aliasStable) continue;

      if (nRLSwitches == 1) {
        // One RL switch. Try the all left and all right aliases
        CDCSegment2D rightSegment = segment;
        CDCSegment2D leftSegment = segment;
        for (CDCRecoHit2D& recoHit2D : rightSegment) {
          if (recoHit2D.getRLInfo() == ERightLeft::c_Left) {
            recoHit2D = recoHit2D.getAlias();
          }
        }

        for (CDCRecoHit2D& recoHit2D : leftSegment) {
          if (recoHit2D.getRLInfo() == ERightLeft::c_Right) {
            recoHit2D = recoHit2D.getAlias();
          }
        }
        refit(rightSegment, true);
        refit(leftSegment, true);
        swapBetterChi2(segment, rightSegment);
        swapBetterChi2(segment, leftSegment);

      } else if (nRLSwitches == 0) {
        // No RL switch. Try to introduce one at the smallest drift length
        CDCSegment2D frontCrossSegment = segment;
        CDCSegment2D backCrossSegment = segment;

        auto lessDriftLength = [](const CDCRecoHit2D & lhs, const CDCRecoHit2D & rhs) {
          return lhs.getRefDriftLength() < rhs.getRefDriftLength();
        };

        // Alias before the minimal drift length
        {
          auto itMinLRecoHit2D =
            std::min_element(frontCrossSegment.begin(), frontCrossSegment.end(), lessDriftLength);
          for (CDCRecoHit2D& recoHit2D : asRange(frontCrossSegment.begin(), itMinLRecoHit2D)) {
            recoHit2D = recoHit2D.getAlias();
          }
        }

        // Alias after the minimal drift length
        {
          auto itMinLRecoHit2D =
            std::min_element(backCrossSegment.begin(), backCrossSegment.end(), lessDriftLength);
          for (CDCRecoHit2D& recoHit2D : asRange(itMinLRecoHit2D, backCrossSegment.end())) {
            recoHit2D = recoHit2D.getAlias();
          }
        }
        refit(frontCrossSegment, true);
        refit(backCrossSegment, true);
        swapBetterChi2(segment, frontCrossSegment);
        swapBetterChi2(segment, backCrossSegment);
      }
    }
  }

  if (m_borderAliases) {
    for (CDCSegment2D& segment : outputSegments) {
      // Check aliasing last hit
      CDCSegment2D aliasSegment = segment;
      CDCRecoHit2D& aliasHit = aliasSegment.back();
      aliasHit.reverse();
      aliasHit.setRecoPos2D(Vector2D(NAN, NAN));
      if (aliasHit.getRefDriftLength() < 0.2) {
        refit(aliasSegment, false);
        Vector2D recoPos2D = aliasSegment.getTrajectory2D().getClosest(aliasHit.getRefPos2D());
        ERightLeft rlInfo = aliasSegment.getTrajectory2D().isRightOrLeft(aliasHit.getRefPos2D());
        aliasHit.setRecoPos2D(recoPos2D);
        aliasHit.setRLInfo(rlInfo);
        swapBetterChi2(segment, aliasSegment);
      }
    }
  }

  if (m_borderAliases) {
    for (CDCSegment2D& segment : outputSegments) {
      // Check aliasing first hit
      CDCSegment2D aliasSegment = segment;
      CDCRecoHit2D& aliasHit = aliasSegment.front();
      aliasHit.reverse();
      aliasHit.setRecoPos2D(Vector2D(NAN, NAN));
      if (aliasHit.getRefDriftLength() < 0.2) {
        refit(aliasSegment, false);
        Vector2D recoPos2D = aliasSegment.getTrajectory2D().getClosest(aliasHit.getRefPos2D());
        ERightLeft rlInfo = aliasSegment.getTrajectory2D().isRightOrLeft(aliasHit.getRefPos2D());
        aliasHit.setRecoPos2D(recoPos2D);
        aliasHit.setRLInfo(rlInfo);
        swapBetterChi2(segment, aliasSegment);
      }
    }
  }

  if (m_middleAliases) {
    for (CDCSegment2D& segment : outputSegments) {
      // Check aliasing hit with lowest drift length
      CDCSegment2D aliasSegment = segment;

      // Find hit with minimal drift length
      double minimalDriftLength = 0.1;
      CDCRecoHit2D* minimalHit = nullptr;
      for (CDCRecoHit2D& aliasHit : aliasSegment) {
        if (aliasHit.getRefDriftLength() < minimalDriftLength) {
          minimalHit = &aliasHit;
          minimalDriftLength = aliasHit.getRefDriftLength();
        }
      }
      if (minimalHit) {
        CDCRecoHit2D& aliasHit = *minimalHit;
        aliasHit.reverse();
        aliasHit.setRecoPos2D(Vector2D(NAN, NAN));
        refit(aliasSegment, false);
        Vector2D recoPos2D = aliasSegment.getTrajectory2D().getClosest(aliasHit.getRefPos2D());
        ERightLeft rlInfo = aliasSegment.getTrajectory2D().isRightOrLeft(aliasHit.getRefPos2D());
        aliasHit.setRecoPos2D(recoPos2D);
        aliasHit.setRLInfo(rlInfo);
        swapBetterChi2(segment, aliasSegment);
      }
    }
  }

  std::sort(outputSegments.begin(), outputSegments.end());
}

void SegmentAliasResolver::refit(CDCSegment2D& segment, bool reestimate)
{
  if (reestimate) {
    if (m_param_reestimatePositions) {
      CDCRLWireHitSegment rlWireHitSegment = segment.getRLWireHitSegment();
      // Reset the drift length to the default values.
      for (CDCRLWireHit& rlWireHit : rlWireHitSegment) {
        rlWireHit.setRefDriftLength(rlWireHit.getWireHit().getRefDriftLength());
      }
      CDCFacetSegment facetSegment = CDCFacetSegment::create(rlWireHitSegment);
      for (CDCFacet& facet : facetSegment) {
        facet.adjustFitLine();
        if (m_param_reestimateDriftLength) {
          m_driftLengthEstimator.updateDriftLength(facet);
        }
      }
      CDCSegment2D replacement = CDCSegment2D::condense(facetSegment);
      segment = replacement;
    }

    if (m_param_reestimateDriftLength) {
      m_driftLengthEstimator.updateDriftLength(segment);
    }
  }

  const EFitPos fitPos = EFitPos::c_RecoPos;
  const EFitVariance fitVariance = EFitVariance::c_Proper;
  CDCObservations2D observations2D(fitPos, fitVariance);
  observations2D.appendRange(segment);
  if (observations2D.size() < 4) {
    segment.getTrajectory2D().clear();
  } else {
    CDCTrajectory2D trajectory2D = m_riemannFitter.fit(observations2D);
    segment.setTrajectory2D(trajectory2D);
  }

}
