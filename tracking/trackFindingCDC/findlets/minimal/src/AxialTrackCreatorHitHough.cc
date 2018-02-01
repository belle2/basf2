/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitHough.h>

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

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

std::string AxialTrackCreatorHitHough::getDescription()
{
  return "Generates axial tracks from hits using several increasingly relaxed hough space search over phi0 and curvature.";
}

void AxialTrackCreatorHitHough::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  // Parameters for the hough space
  moduleParamList->addParameter(prefixed(prefix, "granularityLevel"),
                                m_param_granularityLevel,
                                "Level of divisions in the hough space.",
                                m_param_granularityLevel);

  moduleParamList->addParameter(prefixed(prefix, "sectorLevelSkip"),
                                m_param_sectorLevelSkip,
                                "Number of levels to be skipped in the hough "
                                "space on the first level to form sectors",
                                m_param_sectorLevelSkip);

  moduleParamList->addParameter(prefixed(prefix, "curvBounds"),
                                m_param_curvBounds,
                                "Curvature bounds of the hough space. Either 2 or all discrete bounds",
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

  // Relaxation schedule
  moduleParamList->addParameter(prefixed(prefix, "relaxationSchedule"),
                                m_param_relaxationSchedule,
                                "Relaxation schedule for the leaf processor in the hough tree. "
                                "For content of the individual parameter maps consider the parameters of the "
                                "AxialLegendreLeafProcessor",
                                m_param_relaxationSchedule);

}

void AxialTrackCreatorHitHough::initialize()
{
  Super::initialize();

  // Construct the hough space
  const long nPhi0Bins = std::pow(c_phi0Divisions, m_param_granularityLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_param_discretePhi0Overlap,
                                  m_param_discretePhi0Width);

  if (m_param_curvBounds.size() == 2) {
    // If parameters are unchanged use the legendre default binning
    if (m_param_discreteCurvOverlap == -1) {
      m_param_discreteCurvWidth = 1;
      std::array<float, 2> curvSpan({m_param_curvBounds[0], m_param_curvBounds[1]});
      m_param_curvBounds = this->getDefaultCurvBounds(curvSpan, m_param_granularityLevel);
    } else {
      std::array<double, 2> curvBounds{{m_param_curvBounds.front(), m_param_curvBounds.back()}};
      const long nCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
      const CurvBinsSpec curvBinsSpec(curvBounds.front(),
                                      curvBounds.back(),
                                      nCurvBins,
                                      m_param_discreteCurvOverlap,
                                      m_param_discreteCurvWidth);
      m_param_curvBounds = curvBinsSpec.constructArray();
    }
  }

  // Construct hough tree
  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  m_houghTree = std::make_unique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
  m_houghTree->setSectorLevelSkip(m_param_sectorLevelSkip);
  m_houghTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
  m_houghTree->assignArray<DiscreteCurv>(m_param_curvBounds, m_param_discreteCurvOverlap);
  m_houghTree->initialize();
}

void AxialTrackCreatorHitHough::apply(const std::vector<const CDCWireHit*>& axialWireHits,
                                      std::vector<CDCTrack>& tracks)
{
  // Reset the mask flag and select only the untaken hits
  std::vector<const CDCWireHit*> unusedAxialWireHits;
  for (const CDCWireHit* wireHit : axialWireHits) {
    (*wireHit)->setMaskedFlag(false);
    if ((*wireHit)->hasTakenFlag()) continue;
    unusedAxialWireHits.push_back(wireHit);
  }

  // Setup the level processor and obtain its parameter list to be set.
  using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  AxialLegendreLeafProcessor<Node> leafProcessor(maxTreeLevel);
  leafProcessor.setAxialWireHits(axialWireHits);
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  leafProcessor.exposeParameters(&moduleParamList, prefix);

  // Find tracks with increasingly relaxed conditions in the hough grid
  m_houghTree->seed(std::move(unusedAxialWireHits));
  for (const ParameterVariantMap& passParameters : m_param_relaxationSchedule) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    leafProcessor.beginWalk();
    m_houghTree->findUsing(leafProcessor);
  }
  m_houghTree->fell();

  // Write out tracks as return value
  const std::vector<CDCTrack>& foundTracks = leafProcessor.getTracks();
  tracks.insert(tracks.end(), foundTracks.begin(), foundTracks.end());
}

void AxialTrackCreatorHitHough::terminate()
{
  m_houghTree->raze();
  m_houghTree.reset();
  Super::terminate();
}

std::vector<float> AxialTrackCreatorHitHough::getDefaultCurvBounds(std::array<float, 2> curvSpan, int granularityLevel)
{
  using BinSpan = std::array<float, 2>;
  using BinSpans = std::vector<BinSpan>;
  std::vector<BinSpans> binSpansByLevel(granularityLevel + 1);
  binSpansByLevel[0].push_back(BinSpan({curvSpan[0], curvSpan[1]}));

  for (int level = 1; level <= granularityLevel; ++level) {
    for (const BinSpan& binSpan : binSpansByLevel[level - 1]) {
      const float subBinWidth = std::fabs(binSpan[1] - binSpan[0]) / 2;
      const float middle = binSpan[0] + (binSpan[1] - binSpan[0]) / 2.0;

      // Expaning bins somewhat to have a overlap
      // Assuming granularity level = 12
      // For level 6 to 7 only expand 1 / 4, for higher levels expand  1 / 8.
      // Never expand for curvatures lower than 0.005
      // (copied from the legendre method. Works well, but some experimentation
      //  needs to be made to know why)
      const float extension = [&]() {
        if ((level + 7 <= granularityLevel)
            // or (std::fabs(middle) <= 0.007)
            or (std::fabs(middle) <= 0.005)) {
          return 0.0;
        } else if (level + 5 < granularityLevel) {
          return subBinWidth / 4.0;
        } else {
          return subBinWidth / 8.0;
        }
      }();

      const float lower1 = binSpan[0] - extension;
      const float upper1 = middle + extension;

      const float lower2 = middle - extension;
      const float upper2 = binSpan[1] + extension;

      binSpansByLevel[level].push_back({lower1, upper1});
      binSpansByLevel[level].push_back({lower2, upper2});
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
