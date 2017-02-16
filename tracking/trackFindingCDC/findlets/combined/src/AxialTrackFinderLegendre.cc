/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>

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

#if 0
namespace {
  void saveBounds(std::vector<float> bounds, const std::string& fileName)
  {
    std::ofstream boundsFile;
    boundsFile.open(fileName);
    for (float bound : bounds) {
      boundsFile << bound;
      boundsFile << "\n";
    }
    boundsFile.close();
  }

  std::vector<float> loadBounds(const std::string& fileName)
  {
    std::vector<float> bounds;
    std::ifstream boundsFile;
    std::string boundLine;
    boundsFile.open(fileName);
    if (boundsFile.is_open()) {
      while (std::getline(boundsFile, boundLine)) {
        float bound = stof(boundLine);
        bounds.push_back(bound);
      }
      boundsFile.close();
    } else {
      B2ERROR("Could not read bounds file");
    }
    return bounds;
  }
}
#endif

AxialTrackFinderLegendre::AxialTrackFinderLegendre()
  : Super()
{
  addProcessingSignalListener(&m_axialTrackMerger);
}

std::string AxialTrackFinderLegendre::getDescription()
{
  return "Generates axial tracks from hits using several increasingly relaxed hough space search over phi0 and curvature.";
}

void AxialTrackFinderLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  m_axialTrackMerger.exposeParameters(moduleParamList, prefixed("merge", prefix));

  // Parameters for the fine hough space
  moduleParamList->addParameter(prefixed(prefix, "fineGranularityLevel"),
                                m_param_fineGranularityLevel,
                                "Level of divisions in the fine hough space.",
                                m_param_fineGranularityLevel);

  moduleParamList->addParameter(prefixed(prefix, "fineSectorLevelSkip"),
                                m_param_fineSectorLevelSkip,
                                "Number of levels to be skipped in the fine hough "
                                "space on the first level to form sectors",
                                m_param_fineSectorLevelSkip);

  moduleParamList->addParameter(prefixed(prefix, "fineCurvBounds"),
                                m_param_fineCurvBounds,
                                "Curvature bounds of the fine hough space. Either 2 or all discrete bounds",
                                m_param_fineCurvBounds);

  moduleParamList->addParameter(prefixed(prefix, "fineDiscretePhi0Width"),
                                m_param_fineDiscretePhi0Width,
                                "Width of the phi0 bins at the lowest level of the fine hough space.",
                                m_param_fineDiscretePhi0Width);

  moduleParamList->addParameter(prefixed(prefix, "fineDiscretePhi0Overlap"),
                                m_param_fineDiscretePhi0Overlap,
                                "Overlap of the phi0 bins at the lowest level of the fine hough space.",
                                m_param_fineDiscretePhi0Overlap);

  moduleParamList->addParameter(prefixed(prefix, "fineDiscreteCurvWidth"),
                                m_param_fineDiscreteCurvWidth,
                                "Width of the curvature bins at the lowest level of the fine hough space.",
                                m_param_fineDiscreteCurvWidth);

  moduleParamList->addParameter(prefixed(prefix, "fineDiscreteCurvOverlap"),
                                m_param_fineDiscreteCurvOverlap,
                                "Overlap of the curvature bins at the lowest level of the fine hough space.",
                                m_param_fineDiscreteCurvOverlap);

  moduleParamList->addParameter(prefixed(prefix, "fineRelaxationSchedule"),
                                m_param_fineRelaxationSchedule,
                                "Relaxation schedule for the leaf processor in the fine hough tree. "
                                "For content of the individual parameter maps consider the parameters of the "
                                "AxialLegendreLeafProcessor",
                                m_param_fineRelaxationSchedule);

  // Parameters for the fine hough space
  moduleParamList->addParameter(prefixed(prefix, "roughGranularityLevel"),
                                m_param_roughGranularityLevel,
                                "Level of divisions in the rough hough space.",
                                m_param_roughGranularityLevel);

  moduleParamList->addParameter(prefixed(prefix, "roughSectorLevelSkip"),
                                m_param_roughSectorLevelSkip,
                                "Number of levels to be skipped in the rough hough "
                                "space on the first level to form sectors",
                                m_param_roughSectorLevelSkip);

  moduleParamList->addParameter(prefixed(prefix, "roughCurvBounds"),
                                m_param_roughCurvBounds,
                                "Curvature bounds of the rough hough space. Either 2 or all discrete bounds",
                                m_param_roughCurvBounds);

  moduleParamList->addParameter(prefixed(prefix, "roughDiscretePhi0Width"),
                                m_param_roughDiscretePhi0Width,
                                "Width of the phi0 bins at the lowest level of the rough hough space.",
                                m_param_roughDiscretePhi0Width);

  moduleParamList->addParameter(prefixed(prefix, "roughDiscretePhi0Overlap"),
                                m_param_roughDiscretePhi0Overlap,
                                "Overlap of the phi0 bins at the lowest level of the rough hough space.",
                                m_param_roughDiscretePhi0Overlap);

  moduleParamList->addParameter(prefixed(prefix, "roughDiscreteCurvWidth"),
                                m_param_roughDiscreteCurvWidth,
                                "Width of the curvature bins at the lowest level of the rough hough space.",
                                m_param_roughDiscreteCurvWidth);

  moduleParamList->addParameter(prefixed(prefix, "roughDiscreteCurvOverlap"),
                                m_param_roughDiscreteCurvOverlap,
                                "Overlap of the curvature bins at the lowest level of the rough hough space.",
                                m_param_roughDiscreteCurvOverlap);

  moduleParamList->addParameter(prefixed(prefix, "roughRelaxationSchedule"),
                                m_param_roughRelaxationSchedule,
                                "Relaxation schedule for the leaf processor in the rough hough tree. "
                                "For content of the individual parameter maps consider the parameters of the "
                                "AxialLegendreLeafProcessor",
                                m_param_roughRelaxationSchedule);
}

void AxialTrackFinderLegendre::initialize()
{
  Super::initialize();

  // Construct the fine hough space
  {
    const long nFinePhi0Bins = std::pow(c_phi0Divisions, m_param_fineGranularityLevel);
    const Phi0BinsSpec finePhi0BinsSpec(nFinePhi0Bins,
                                        m_param_fineDiscretePhi0Overlap,
                                        m_param_fineDiscretePhi0Width);
    if (m_param_fineCurvBounds.size() == 2) {
      // If parameters are unchanged use the legendre default binning
      if (m_param_fineDiscreteCurvOverlap == -1) {
        m_param_fineDiscreteCurvWidth = 1;
        std::array<float, 2> fineCurvSpan({m_param_fineCurvBounds[0], m_param_fineCurvBounds[1]});
        m_param_fineCurvBounds = this->getDefaultCurvBounds(fineCurvSpan, m_param_fineGranularityLevel);
      } else {
        std::array<double, 2> fineCurvBounds{{m_param_fineCurvBounds.front(), m_param_fineCurvBounds.back()}};
        const long nFineCurvBins = std::pow(c_curvDivisions, m_param_fineGranularityLevel);
        const CurvBinsSpec fineCurvBinsSpec(fineCurvBounds.front(),
                                            fineCurvBounds.back(),
                                            nFineCurvBins,
                                            m_param_fineDiscreteCurvOverlap,
                                            m_param_fineDiscreteCurvWidth);
        m_param_fineCurvBounds = fineCurvBinsSpec.constructArray();
      }
    }

    // Construct fine hough tree
    int fineMaxTreeLevel = m_param_fineGranularityLevel - m_param_fineSectorLevelSkip;
    m_fineHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(fineMaxTreeLevel, m_curlCurv);
    m_fineHoughTree->setSectorLevelSkip(m_param_fineSectorLevelSkip);
    m_fineHoughTree->assignArray<DiscretePhi0>(finePhi0BinsSpec.constructArray(), finePhi0BinsSpec.getNOverlap());
    m_fineHoughTree->assignArray<DiscreteCurv>(m_param_fineCurvBounds, m_param_fineDiscreteCurvOverlap);
    m_fineHoughTree->initialize();
  }

  // Construct the rough hough space
  {
    const long nRoughPhi0Bins = std::pow(c_phi0Divisions, m_param_roughGranularityLevel);
    const Phi0BinsSpec roughPhi0BinsSpec(nRoughPhi0Bins,
                                         m_param_roughDiscretePhi0Overlap,
                                         m_param_roughDiscretePhi0Width);
    if (m_param_roughCurvBounds.size() == 2) {
      // If parameters are unchanged use the legendre default binning
      if (m_param_roughDiscreteCurvOverlap == -1) {
        m_param_roughDiscreteCurvWidth = 1;
        std::array<float, 2> roughCurvSpan({m_param_roughCurvBounds[0], m_param_roughCurvBounds[1]});
        m_param_roughCurvBounds = this->getDefaultCurvBounds(roughCurvSpan, m_param_roughGranularityLevel);
      } else {
        std::array<double, 2> roughCurvBounds{{m_param_roughCurvBounds.front(), m_param_roughCurvBounds.back()}};
        const long nRoughCurvBins = std::pow(c_curvDivisions, m_param_roughGranularityLevel);
        const CurvBinsSpec roughCurvBinsSpec(roughCurvBounds.front(),
                                             roughCurvBounds.back(),
                                             nRoughCurvBins,
                                             m_param_roughDiscreteCurvOverlap,
                                             m_param_roughDiscreteCurvWidth);
        m_param_roughCurvBounds = roughCurvBinsSpec.constructArray();
      }
    }

    // Construct rough hough tree
    int roughMaxTreeLevel = m_param_roughGranularityLevel - m_param_roughSectorLevelSkip;
    m_roughHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(roughMaxTreeLevel, m_curlCurv);
    m_roughHoughTree->setSectorLevelSkip(m_param_roughSectorLevelSkip);
    m_roughHoughTree->assignArray<DiscretePhi0>(roughPhi0BinsSpec.constructArray(), roughPhi0BinsSpec.getNOverlap());
    m_roughHoughTree->assignArray<DiscreteCurv>(m_param_roughCurvBounds, m_param_roughDiscreteCurvOverlap);
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

void AxialTrackFinderLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                     std::vector<CDCTrack>& tracks)
{
  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    wireHit->unsetTemporaryFlags();
    wireHit->unsetMaskedFlag();
    if (not wireHit.isAxial()) continue;
    if (wireHit->hasBackgroundFlag()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // Setup the level processor and obtain its parameter list to be set.
  using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
  int maxTreeLevel = m_param_fineGranularityLevel - m_param_fineSectorLevelSkip;
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
  m_fineHoughTree->fell();

  // One step of migrating hits between the already found tracks
  leafProcessor.migrateHits();

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

  // Write out tracks as return value
  const std::vector<CDCTrack>& foundTracks = leafProcessor.getTracks();
  tracks.insert(tracks.end(), foundTracks.begin(), foundTracks.end());

  // Do track merging and finalization steps
  m_axialTrackMerger.apply(tracks, axialWireHits);
}

void AxialTrackFinderLegendre::terminate()
{
  m_fineHoughTree->raze();
  m_fineHoughTree.reset();
  m_roughHoughTree->raze();
  m_roughHoughTree.reset();
  Super::terminate();
}

std::vector<ParameterVariantMap>
AxialTrackFinderLegendre::getDefaultFineRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;
  // Relaxation schedule of the original legendre implemenation
  // Augmented by the road search parameters
  // Note: distinction between integer and double literals is essential
  // For the record: the setting seem a bit non-sensical, but work kind of well, experimentation needed.

  // NonCurler pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_fineSectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 3 - m_param_fineSectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_fineSectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 3 - m_param_fineSectorLevelSkip},
  });

  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 12 - m_param_fineSectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.07},
      {"curvResolution", std::string("origin")},
      {"nRoadSearches", 1},
      {"roadLevel", 3 - m_param_fineSectorLevelSkip},
    });
  }

  // NonCurlerWithIncreasedThreshold pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_fineSectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 3 - m_param_fineSectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_fineSectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 3 - m_param_fineSectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_fineSectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 0.07},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 3 - m_param_fineSectorLevelSkip},
  });

  for (double minWeight = 37.5; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_fineSectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv",  m_maxCurvAcceptance},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 2},
      {"roadLevel", 3 - m_param_fineSectorLevelSkip},
    });
  }

  return result;
}

std::vector<ParameterVariantMap>
AxialTrackFinderLegendre::getDefaultRoughRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;

  // FullRange pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_roughSectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_roughSectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  for (double minWeight = 30.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_roughSectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.15},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 3},
      {"roadLevel", 0},
    });
  }

  return result;
}

std::vector<float> AxialTrackFinderLegendre::getDefaultCurvBounds(std::array<float, 2> curvSpan, int granularityLevel)
{
  using BinSpan = std::array<double, 2>;
  using BinSpans = std::vector<BinSpan>;
  std::vector<BinSpans> binSpansByLevel(granularityLevel + 1);

  binSpansByLevel[0].push_back(BinSpan({curvSpan[0], curvSpan[1]}));

  for (int level = 0; level < granularityLevel; ++level) {
    for (BinSpan& binSpan : binSpansByLevel[level]) {
      double subBinWidth = std::fabs(binSpan[1] - binSpan[0]) / 2;
      double middle = (binSpan[0] + binSpan[1]) / 2;

      // Expaning bins somewhat to have a overlap
      // Assuming granularity level = 12
      // For level 6 to 7 only expand 1 / 4, for higher levels expand  1 / 8.
      // Never expand for curvatures lower than 0.005
      // (copied from the legendre method. Works well, but some experimentation
      //  needs to be made to know why)
      double extension = 0;
      if ((level + 7 <= granularityLevel)
          // or (std::fabs(middle) <= 0.007)
          or (std::fabs(middle) <= 0.005)
         ) {
        extension = 0;
      } else if (level + 5 < granularityLevel) {
        extension = subBinWidth / 4.0;
      } else {
        extension = subBinWidth / 8.0;
      }

      double lower1 = binSpan[0];
      double upper1 = middle + extension;

      double lower2 = middle - extension;
      double upper2 = binSpan[1];

      binSpansByLevel[level + 1].push_back({lower1, upper1});
      binSpansByLevel[level + 1].push_back({lower2, upper2});
    }
  }

  // Return highest level as prepared bin bounds.
  std::vector<float> result;

  for (BinSpan& binSpan : binSpansByLevel[granularityLevel]) {
    result.push_back(binSpan[0]);
    result.push_back(binSpan[1]);
  }
  return result;
}
