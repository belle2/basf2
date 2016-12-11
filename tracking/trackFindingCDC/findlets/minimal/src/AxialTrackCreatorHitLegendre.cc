/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitLegendre.h>

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackCreatorHitLegendre::getDescription()
{
  return "Generates axial tracks from segments using a hough space over phi0 impact and curvature for the spares case.";
}

void AxialTrackCreatorHitLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                    const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "granularityLevel"),
                                m_param_granularityLevel,
                                "Level of divisions in the hough space.",
                                m_param_granularityLevel);

  moduleParamList->addParameter(prefixed(prefix, "sectorLevelSkip"),
                                m_param_sectorLevelSkip,
                                "Number of levels to be skipped on the first level to form sectors",
                                m_param_sectorLevelSkip);

  moduleParamList->addParameter(prefixed(prefix, "curvBounds"),
                                m_param_curvBounds,
                                "Curvature bounds of the hough space.",
                                m_param_curvBounds);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Width"),
                                m_param_discretePhi0Width,
                                "Width of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Width);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Overlap"),
                                m_param_discretePhi0Overlap,
                                "Overlap of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Overlap);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvWidth"),
                                m_param_discreteCurvWidth,
                                "Width of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvWidth);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvOverlap"),
                                m_param_discreteCurvOverlap,
                                "Overlap of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvOverlap);

  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Absolute minimal number of hits to make an axial track.",
                                m_param_minNHits);
}

void AxialTrackCreatorHitLegendre::initialize()
{
  Super::initialize();

  B2ASSERT("Need exactly two curv bounds", m_param_curvBounds.size() == 2);

  const size_t nPhi0Bins = std::pow(c_phi0Divisions, m_param_granularityLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_param_discretePhi0Overlap,
                                  m_param_discretePhi0Width);

  std::array<double, 2> curvBounds{{m_param_curvBounds.front(), m_param_curvBounds.back()}};
  const size_t nCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
  const CurvBinsSpec curvBinsSpec(curvBounds[0],
                                  curvBounds[1],
                                  nCurvBins,
                                  m_param_discreteCurvOverlap,
                                  m_param_discreteCurvWidth);

  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  m_houghTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
  m_houghTree->setSectorLevelSkip(m_param_sectorLevelSkip);
  m_houghTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
  m_houghTree->assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
  m_houghTree->initialize();
}

void AxialTrackCreatorHitLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                         std::vector<CDCTrack>& tracks)
{
  // Acquire the axial hits
  size_t nAxialHits = 0;
  std::vector<CDCRLWireHit> axialRLWireHits;
  axialRLWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit.isAxial()) continue;
    axialRLWireHits.emplace_back(&wireHit);
    ++nAxialHits;
  }

  m_houghTree->seed(std::move(axialRLWireHits));

  using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  AxialLegendreLeafProcessor<Node> leafProcessor(maxTreeLevel);
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  leafProcessor.exposeParameters(&moduleParamList, prefix);

  /// Find tracks with increasingly relaxed conditions
  for (const ParameterVariantMap& passParameters : getRelaxationSchedule()) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    m_houghTree->findUsing(leafProcessor);
  }

  // Pick up the found candidates and make tracks from them
  std::vector<std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit>>> candidates =
    leafProcessor.getCandidates();

  for (const std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit>>& candidate : candidates) {
    const CDCTrajectory2D& trajectory2D = candidate.first;
    const std::vector<CDCRLWireHit>& foundRLWireHits = candidate.second;
    CDCTrack track;
    for (const CDCRLWireHit& rlWireHit : foundRLWireHits) {
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(&rlWireHit.getWireHit(), trajectory2D);
      // CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
      track.push_back(recoHit3D);
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

void AxialTrackCreatorHitLegendre::terminate()
{
  m_houghTree->raze();
  m_houghTree.reset();
  Super::terminate();
}

std::vector<ParameterVariantMap>
AxialTrackCreatorHitLegendre::getRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;
  // Relaxation schedule of the original legendre implemenation
  // Augmented by the road search parameters
  // Note: distinction between integer and float literals is necessary
  // For the record: the setting seem a bit non-sensical

  // NonCurler pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    // {"offOriginPrecision", false},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    // {"offOriginPrecision", false},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},

  });

  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 12 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.07},
      // {"offOriginPrecision", false},
      {"nRoadSearches", 2},
      {"roadLevel", 4 - m_param_sectorLevelSkip},
    });
  }

  // Skipping other passes for the moment
  return result;

  // NonCurlerWithIncreasedThreshold pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    // {"offOriginPrecision", true},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    // {"offOriginPrecision", true},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},

  });

  int iPass = 0;
  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", iPass == 0 ? 0.07 : 0.14},
      // {"offOriginPrecision", true},
      {"nRoadSearches", 2},
      {"roadLevel", 4 - m_param_sectorLevelSkip},
    });
    ++iPass;
  }

  // FullRange pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    // {"offOriginPrecision", true},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    // {"offOriginPrecision", true},
    {"nRoadSearches", 3},
    {"roadLevel", 0},

  });

  for (double minWeight = 30.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.15},
      // {"offOriginPrecision", true},
      {"nRoadSearches", 3},
      {"roadLevel", 0},
    });
  }

  return result;
}
