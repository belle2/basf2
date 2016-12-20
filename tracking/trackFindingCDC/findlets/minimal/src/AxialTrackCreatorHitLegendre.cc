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

#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/TrackMerger.h>
#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

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

  moduleParamList->addParameter(prefixed(prefix, "fineCurvBounds"),
                                m_param_fineCurvBounds,
                                "Curvature bounds of the fine hough space. Either 2 or all discrete bounds",
                                m_param_fineCurvBounds);

  moduleParamList->addParameter(prefixed(prefix, "roughCurvBounds"),
                                m_param_roughCurvBounds,
                                "Curvature bounds of the rough hough space. Either 2 or all discrete bounds",
                                m_param_roughCurvBounds);

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

  moduleParamList->addParameter(prefixed(prefix, "fineRelaxationSchedule"),
                                m_param_fineRelaxationSchedule,
                                "Relaxation schedule for the leaf processor in the fine hough tree. "
                                "For content of the individual parameter maps consider the parameters of the "
                                "AxialLegendreLeafProcessor",
                                m_param_fineRelaxationSchedule);

  moduleParamList->addParameter(prefixed(prefix, "roughRelaxationSchedule"),
                                m_param_roughRelaxationSchedule,
                                "Relaxation schedule for the leaf processor in the rough hough tree. "
                                "For content of the individual parameter maps consider the parameters of the "
                                "AxialLegendreLeafProcessor",
                                m_param_roughRelaxationSchedule);
}

void AxialTrackCreatorHitLegendre::initialize()
{
  Super::initialize();
  const size_t nPhi0Bins = std::pow(c_phi0Divisions, m_param_granularityLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_param_discretePhi0Overlap,
                                  m_param_discretePhi0Width);

  // Construct the fine curvature array in case two bounds were given
  if (m_param_fineCurvBounds.size() == 2) {

    std::array<double, 2> fineCurvBounds{{m_param_fineCurvBounds.front(), m_param_fineCurvBounds.back()}};
    const size_t nFineCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
    const CurvBinsSpec fineCurvBinsSpec(fineCurvBounds.front(),
                                        fineCurvBounds.back(),
                                        nFineCurvBins,
                                        m_param_discreteCurvOverlap,
                                        m_param_discreteCurvWidth);
    m_param_fineCurvBounds = fineCurvBinsSpec.constructArray();
  }

  // Construct the rough curvature array in case two bounds were given
  if (m_param_roughCurvBounds.size() == 2) {
    std::array<double, 2> roughCurvBounds{{m_param_roughCurvBounds.front(), m_param_roughCurvBounds.back()}};
    const size_t nRoughCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
    const CurvBinsSpec roughCurvBinsSpec(roughCurvBounds.front(),
                                         roughCurvBounds.back(),
                                         nRoughCurvBins,
                                         m_param_discreteCurvOverlap,
                                         m_param_discreteCurvWidth);
    m_param_roughCurvBounds = roughCurvBinsSpec.constructArray();
  }

  // Construct fine hough tree
  {
    int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
    m_fineHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
    m_fineHoughTree->setSectorLevelSkip(m_param_sectorLevelSkip);
    m_fineHoughTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    m_fineHoughTree->assignArray<DiscreteCurv>(m_param_fineCurvBounds, m_param_discreteCurvOverlap);
    m_fineHoughTree->initialize();
  }

  // Construct rough hough tree
  {
    int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
    m_roughHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
    // No level skip !
    m_roughHoughTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    m_roughHoughTree->assignArray<DiscreteCurv>(m_param_roughCurvBounds, m_param_discreteCurvOverlap);
    m_roughHoughTree->initialize();
  }

  // If schedule is not given externally use default one
  if (m_param_fineRelaxationSchedule.empty()) {
    m_param_fineRelaxationSchedule = getDefaultFineRelaxationSchedule();
  }

  // If schedule is not given externally use default one
  if (m_param_roughRelaxationSchedule.empty()) {
    m_param_roughRelaxationSchedule = getDefaultRoughRelaxationSchedule();
  }
}

void AxialTrackCreatorHitLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                         std::vector<CDCTrack>& tracks)
{
  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit.isAxial()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // Setup the level processor and obtain its parameter list to be set.
  using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  AxialLegendreLeafProcessor<Node> leafProcessor(maxTreeLevel);
  leafProcessor.setAxialWireHits(axialWireHits);
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  leafProcessor.exposeParameters(&moduleParamList, prefix);

  // Find tracks with increasingly relaxed conditions in the fine hough grid
  m_fineHoughTree->seed(leafProcessor.getUnusedWireHits());
  for (const ParameterVariantMap& passParameters : m_param_fineRelaxationSchedule) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    leafProcessor.beginWalk();
    m_fineHoughTree->findUsing(leafProcessor);
  }

  /*
  B2WARNING("Node calls " << leafProcessor.m_nNodes);
  B2WARNING("Skipped nodes calls " << leafProcessor.m_nSkippedNodes);
  B2WARNING("Leaf calls " << leafProcessor.m_nLeafs);

  int usedFlag = 0;
  int maskedFlag = 0;
  for (const CDCWireHit* wireHit : axialWireHits) {
    const AutomatonCell automatonCell = wireHit->getAutomatonCell();
    if (automatonCell.hasMaskedFlag()) ++maskedFlag;
    if (automatonCell.hasMaskedFlag() or automatonCell.hasTakenFlag()) ++usedFlag;
  }
  B2WARNING("usedFlag " << usedFlag);
  B2WARNING("maskedFlag " << maskedFlag);
  */

  m_fineHoughTree->fell();

  // One step of migrating hits between the already found tracks
  leafProcessor.migrateHits();

  /*
  // Find tracks with increasingly relaxed conditions in the rougher hough grid
  m_roughHoughTree->seed(leafProcessor.getUnusedWireHits());
  for (const ParameterVariantMap& passParameters : m_param_roughRelaxationSchedule) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    leafProcessor.beginWalk();
    m_roughHoughTree->findUsing(leafProcessor);
  }
  m_roughHoughTree->fell();

  // One step of migrating hits between the already found tracks
  leafProcessor.migrateHits();
  */

  leafProcessor.finalizeTracks();

  const std::list<CDCTrack>& foundTracks = leafProcessor.getTracks();
  tracks.insert(tracks.end(), foundTracks.begin(), foundTracks.end());
  return;

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
  m_fineHoughTree->raze();
  m_fineHoughTree.reset();
  m_roughHoughTree->raze();
  m_roughHoughTree.reset();
  Super::terminate();
}

std::vector<ParameterVariantMap>
AxialTrackCreatorHitLegendre::getDefaultFineRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;
  // Relaxation schedule of the original legendre implemenation
  // Augmented by the road search parameters
  // Note: distinction between integer and double literals is essential
  // For the record: the setting seem a bit non-sensical

  // NonCurler pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 12 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.07},
      {"curvResolution", std::string("origin")},
      {"nRoadSearches", 1},
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
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", m_maxCurvAcceptance / 2},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  for (double minWeight = 37.5; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv",  m_maxCurvAcceptance},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 2},
      {"roadLevel", 4 - m_param_sectorLevelSkip},
    });
  }
  return result;
}

std::vector<ParameterVariantMap>
AxialTrackCreatorHitLegendre::getDefaultRoughRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;

  // FullRange pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  for (double minWeight = 30.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10},
      {"minWeight", minWeight},
      {"maxCurv", 0.15},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 3},
      {"roadLevel", 0},
    });
  }

  return result;
}
