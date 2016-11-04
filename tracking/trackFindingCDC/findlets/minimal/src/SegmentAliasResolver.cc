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
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  void swapBetterChi2(CDCRecoSegment2D& segment, CDCRecoSegment2D& aliasSegment)
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
  moduleParamList->addParameter(prefixed(prefix, "investigate"),
                                m_param_investigate,
                                "Which alias resolutions should be applied. "
                                "Options are trailing_hit, last_hit, full.",
                                m_param_investigate);

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
  for (const std::string& investigatedAlias : m_param_investigate) {
    if (investigatedAlias == "full") {
      m_fullAlias = true;
    } else if (investigatedAlias == "borders") {
      m_borderAliases = true;
    } else if (investigatedAlias == "middle") {
      m_middleAliases = true;
    } else {
      B2ERROR("Unknown alias to investigate " << investigatedAlias);
    }
  }
}

void SegmentAliasResolver::apply(std::vector<CDCRecoSegment2D>& outputSegments)
{
  if (m_fullAlias) {
    for (CDCRecoSegment2D& segment : outputSegments) {
      int nRLSwitches = segment.getNRLSwitches();
      // Sufficiently right left constrained that the alias is already fixed.
      bool aliasStable = nRLSwitches > 2;
      if (aliasStable) continue;

      CDCRecoSegment2D aliasSegment = segment.getAlias();
      refit(aliasSegment, true);
      swapBetterChi2(segment, aliasSegment);
    } // end alias loop
  } // end if alias full

  if (m_borderAliases) {
    for (CDCRecoSegment2D& segment : outputSegments) {
      // Check aliasing last hit
      CDCRecoSegment2D aliasSegment = segment;
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
    for (CDCRecoSegment2D& segment : outputSegments) {
      // Check aliasing first hit
      CDCRecoSegment2D aliasSegment = segment;
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
    for (CDCRecoSegment2D& segment : outputSegments) {
      // Check aliasing hit with lowest drift length
      CDCRecoSegment2D aliasSegment = segment;

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
}

void SegmentAliasResolver::refit(CDCRecoSegment2D& segment, bool reestimate)
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
          for (CDCFacet& facet : facetSegment) {
            m_driftLengthEstimator.updateDriftLength(facet);
          }
        }
      }
      CDCRecoSegment2D replacement = CDCRecoSegment2D::condense(facetSegment);
      segment = replacement;
    }

    if (m_param_reestimateDriftLength) {
      m_driftLengthEstimator.updateDriftLength(segment);
    }
  }

  EFitPos fitPos = EFitPos::c_RecoPos;
  EFitVariance fitVariance = EFitVariance::c_Proper;
  CDCObservations2D observations2D(fitPos, fitVariance);
  observations2D.appendRange(segment);
  if (observations2D.size() < 4) {
    segment.getTrajectory2D().clear();
  } else {
    CDCTrajectory2D trajectory2D = m_riemannFitter.fit(observations2D);
    segment.setTrajectory2D(trajectory2D);
  }

}
