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

AxialTrackFinderLegendre::AxialTrackFinderLegendre()
  : Super()
{
  addProcessingSignalListener(&m_fineLegendreSearch);
  addProcessingSignalListener(&m_roughLegendreSearch);
  addProcessingSignalListener(&m_axialTrackMerger);

  // Set default parameters of the hough spaces
  /////////////////////////////////////////////

  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);

  // Setup the default parameters of the fine hough space
  moduleParamList.getParameter<int>("fineGranularityLevel").setDefaultValue(12);
  moduleParamList.getParameter<int>("fineSectorLevelSkip").setDefaultValue(2);
  // moduleParamList.getParameter<std::vector<double>>("fineCurvBounds").setDefaultValue({ -0.018, 0.75});
  moduleParamList.getParameter<std::vector<float>>("fineCurvBounds").setDefaultValue({{ -0.02, 0.14}});
  moduleParamList.getParameter<int>("fineDiscretePhi0Width").setDefaultValue(19);
  moduleParamList.getParameter<int>("fineDiscretePhi0Overlap").setDefaultValue(5);
  moduleParamList.getParameter<int>("fineDiscreteCurvWidth").setDefaultValue(1);
  moduleParamList.getParameter<int>("fineDiscreteCurvOverlap").setDefaultValue(-1);
  moduleParamList.getParameter<std::vector<ParameterVariantMap>>("fineRelaxationSchedule")
                                                              .setDefaultValue(getDefaultFineRelaxationSchedule());

  // Setup the default parameters of the rough hough space
  moduleParamList.getParameter<int>("roughGranularityLevel").setDefaultValue(10);
  moduleParamList.getParameter<int>("roughSectorLevelSkip").setDefaultValue(0);
  moduleParamList.getParameter<std::vector<float>>("roughCurvBounds").setDefaultValue({{ 0.0, 0.30}});
  moduleParamList.getParameter<int>("roughDiscretePhi0Width").setDefaultValue(19);
  moduleParamList.getParameter<int>("roughDiscretePhi0Overlap").setDefaultValue(5);
  moduleParamList.getParameter<int>("roughDiscreteCurvWidth").setDefaultValue(1);
  moduleParamList.getParameter<int>("roughDiscreteCurvOverlap").setDefaultValue(-1);
  moduleParamList.getParameter<std::vector<ParameterVariantMap>>("roughRelaxationSchedule")
                                                              .setDefaultValue(getDefaultRoughRelaxationSchedule());
}

std::string AxialTrackFinderLegendre::getDescription()
{
  return "Generates axial tracks from hits using several increasingly relaxed hough space search over phi0 and curvature.";
}

void AxialTrackFinderLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  m_fineLegendreSearch.exposeParameters(moduleParamList, prefixed("fine", prefix));
  m_roughLegendreSearch.exposeParameters(moduleParamList, prefixed("rough", prefix));
  m_axialTrackMerger.exposeParameters(moduleParamList, prefixed("merge", prefix));
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

  // Fine legendre search
  m_fineLegendreSearch.apply(axialWireHits, tracks);

  // One step of migrating hits between the already found tracks
  TrackProcessor::assignNewHits(axialWireHits, tracks);

  // Rough legendre search
  m_roughLegendreSearch.apply(axialWireHits, tracks);

  // One step of migrating hits between the already found tracks
  TrackProcessor::assignNewHits(axialWireHits, tracks);

  // Do track merging and finalization steps
  m_axialTrackMerger.apply(tracks, axialWireHits);
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
    {"maxLevel", 10},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * 0.02},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 1},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * 0.02},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 1},
  });

  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10},
      {"minWeight", minWeight},
      {"maxCurv", 0.07},
      {"curvResolution", std::string("origin")},
      {"nRoadSearches", 1},
      {"roadLevel", 1},
    });
  }

  // NonCurlerWithIncreasedThreshold pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 8},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * 0.02},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 1},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 8},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * 0.02},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 1},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 8},
    {"minWeight", 50.0},
    {"maxCurv", 0.07},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 1},
  });

  for (double minWeight = 37.5; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 8},
      {"minWeight", minWeight},
      {"maxCurv",  m_maxCurvAcceptance},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 2},
      {"roadLevel", 1},
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
    {"maxLevel", 10},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * 0.02},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * 0.02},
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
