/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorSegmentHough.h>

#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackCreatorSegmentHough::getDescription()
{
  return "Generates axial tracks from segments using a hough space over phi0 impact and curvature for the spares case.";
}

void AxialTrackCreatorSegmentHough::exposeParameters(ModuleParamList* moduleParamList,
                                                     const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Absolute minimal number of hits to make an axial track.",
                                m_param_minNHits);

  moduleParamList->addParameter(prefixed(prefix, "minFractionNHits"),
                                m_param_minFractionNHits,
                                "Minimal number of hits as a fraction of the total hits in the event.",
                                m_param_minFractionNHits);

  moduleParamList->addParameter(prefixed(prefix, "maxLevel"),
                                m_param_maxLevel,
                                "Level of divisions in the hough space.",
                                m_param_maxLevel);

  moduleParamList->addParameter(prefixed(prefix, "curvBounds"),
                                m_param_curvBounds,
                                "Curvature bounds of the hough space.",
                                m_param_curvBounds);

  moduleParamList->addParameter(prefixed(prefix, "impactBounds"),
                                m_param_impactBounds,
                                "Impact parameter bounds of the hough space.",
                                m_param_impactBounds);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Width"),
                                m_param_discretePhi0Width,
                                "Width of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Width);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Overlap"),
                                m_param_discretePhi0Overlap,
                                "Overlap of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Overlap);

  moduleParamList->addParameter(prefixed(prefix, "discreteImpactWidth"),
                                m_param_discreteImpactWidth,
                                "Width of the impact bins at the lowest level of the hough space.",
                                m_param_discreteImpactWidth);

  moduleParamList->addParameter(prefixed(prefix, "discreteImpactOverlap"),
                                m_param_discreteImpactOverlap,
                                "Overlap of the impact bins at the lowest level of the hough space.",
                                m_param_discreteImpactOverlap);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvWidth"),
                                m_param_discreteCurvWidth,
                                "Width of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvWidth);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvOverlap"),
                                m_param_discreteCurvOverlap,
                                "Overlap of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvOverlap);

}

void AxialTrackCreatorSegmentHough::initialize()
{
  Super::initialize();

  B2ASSERT("Need exactly two curv bound", m_param_curvBounds.size() == 2);
  B2ASSERT("Need exactly two impact bound", m_param_impactBounds.size() == 2);

  const size_t nPhi0Bins = std::pow(c_phi0Divisions, m_param_maxLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_param_discretePhi0Overlap,
                                  m_param_discretePhi0Width);

  std::array<double, 2> impactBounds{{m_param_impactBounds.front(), m_param_impactBounds.back()}};
  const size_t nImpactBins = std::pow(c_impactDivisions, m_param_maxLevel);
  const ImpactBinsSpec impactBinsSpec(impactBounds[0],
                                      impactBounds[1],
                                      nImpactBins,
                                      m_param_discreteImpactOverlap,
                                      m_param_discreteImpactWidth);

  std::array<double, 2> curvBounds{{m_param_curvBounds.front(), m_param_curvBounds.back()}};
  const size_t nCurvBins = std::pow(c_curvDivisions, m_param_maxLevel);
  const CurvBinsSpec curvBinsSpec(curvBounds[0],
                                  curvBounds[1],
                                  nCurvBins,
                                  m_param_discreteCurvOverlap,
                                  m_param_discreteCurvWidth);

  m_houghTree = std::make_unique<SimpleSegmentPhi0ImpactCurvHoughTree>(m_param_maxLevel);
  m_houghTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
  m_houghTree->assignArray<ContinuousImpact>(impactBounds, impactBinsSpec.getOverlap()); // Continuous
  m_houghTree->assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
  m_houghTree->initialize();
}

void AxialTrackCreatorSegmentHough::apply(const std::vector<CDCSegment2D>& segments,
                                          std::vector<CDCTrack>& tracks)
{
  m_houghTree->fell();

  size_t nAxialHits = 0;
  std::vector<const CDCSegment2D*> ptrAxialSegments;
  ptrAxialSegments.reserve(segments.size());

  for (const CDCSegment2D& segment : segments) {
    if (segment.getStereoKind() == EStereoKind::c_Axial) {
      ptrAxialSegments.push_back(&segment);
      nAxialHits += segment.size();
    }
  }

  m_houghTree->seed(ptrAxialSegments);
  using HoughBox = SimpleSegmentPhi0ImpactCurvHoughTree::HoughBox;

  Weight minWeight = std::min(m_param_minNHits, nAxialHits * m_param_minFractionNHits);

  using Candidate = std::pair<HoughBox, std::vector<const CDCSegment2D*> >;
  std::vector<Candidate> candidates = m_houghTree->findBest(minWeight);

  for (const Candidate& candidate : candidates) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    CDCObservations2D observations;

    const HoughBox& foundHoughBox = candidate.first;
    const std::vector<const CDCSegment2D*>& foundSegments = candidate.second;
    for (const CDCSegment2D* segment : foundSegments) {
      observations.appendRange(*segment);
    }
    CDCTrajectory2D trajectory2D = fitter.fit(observations);

    // Check if the circle has been fitted reverse to the hough box by accident
    {
      double curv = trajectory2D.getCurvature();
      const std::array<DiscreteCurv, 2>& curvs = foundHoughBox.getBounds<DiscreteCurv>();
      float lowerCurv = *(curvs[0]);
      float upperCurv = *(curvs[1]);
      if (ESignUtil::common(lowerCurv, upperCurv) * curv < 0) {
        trajectory2D.reverse();
      }
    }

    CDCTrack track;
    for (const CDCSegment2D* segment : foundSegments) {
      for (const CDCRecoHit2D& recoHit2D : *segment) {
        track.push_back(CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D));
      }
    }
    track.sortByArcLength2D();

    /// Setting trajectories
    if (track.empty()) continue;
    const CDCRecoHit3D& startRecoHit3D = track.front();
    CDCTrajectory3D startTrajectory3D(trajectory2D);
    startTrajectory3D.setLocalOrigin(startRecoHit3D.getRecoPos3D());
    track.setStartTrajectory3D(startTrajectory3D);

    const CDCRecoHit3D& endRecoHit3D = track.back();
    CDCTrajectory3D endTrajectory3D(trajectory2D);
    endTrajectory3D.setLocalOrigin(endRecoHit3D.getRecoPos3D());
    track.setEndTrajectory3D(endTrajectory3D);

    tracks.push_back(std::move(track));
  }
}

void AxialTrackCreatorSegmentHough::terminate()
{
  m_houghTree->raze();
  m_houghTree.reset();
  Super::terminate();
}
