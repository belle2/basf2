/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/Styling.h>
#include <tracking/trackFindingCDC/display/HitColorMapping.h>
#include <tracking/trackFindingCDC/display/SegmentColorMapping.h>
#include <tracking/trackFindingCDC/display/RecoTrackColorMapping.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::unique_ptr<ChooseableRecoTrackStyling::ObjectMapping>
ChooseableRecoTrackStyling::createMapping(const std::string& mappingName)
{
  if (mappingName == "ListColors") {
    return std::make_unique<RecoTrackListColorsColorMap>();
  } else if (mappingName == "MatchingStatus") {
    return std::make_unique<RecoTrackMatchingStatusColorMap>();
  } else if (mappingName == "MCMatchingStatus") {
    return std::make_unique<MCRecoTrackMatchingStatusColorMap>();
  } else {
    return Super::createMapping(mappingName);
  }
}

std::unique_ptr<ChooseableSegmentStyling::ObjectMapping>
ChooseableSegmentStyling::createMapping(const std::string& mappingName)
{
  if (mappingName == "SegmentMCTrackIdColorMap") {
    return std::make_unique<SegmentMCTrackIdColorMap>();
  } else if (mappingName == "SegmentFBInfoColorMap") {
    return std::make_unique<SegmentFBInfoColorMap>();
  } else if (mappingName == "SegmentFirstInTrackIdColorMap") {
    return std::make_unique<SegmentFirstInTrackIdColorMap>();
  } else if (mappingName == "SegmentLastInTrackIdColorMap") {
    return std::make_unique<SegmentLastInTrackIdColorMap>();
  } else if (mappingName == "SegmentFirstNPassedSuperLayersColorMap") {
    return std::make_unique<SegmentFirstNPassedSuperLayersColorMap>();
  } else if (mappingName == "SegmentLastNPassedSuperLayersColorMap") {
    return std::make_unique<SegmentLastNPassedSuperLayersColorMap>();
  } else if (mappingName == "ListColors") {
    return std::make_unique<SegmentListColorsColorMap>();
  } else {
    return Super::createMapping(mappingName);
  }
}

std::unique_ptr<ChooseableHitStyling::ObjectMapping>
ChooseableHitStyling::createMapping(const std::string& mappingName)
{
  if (mappingName == "ZeroDriftLengthColorMap") {
    return std::make_unique<ZeroDriftLengthColorMap>();
  } else if (mappingName == "ZeroDriftLengthStrokeWidthMap") {
    return std::make_unique<ZeroDriftLengthStrokeWidthMap>();
  } else if (mappingName == "TakenFlagColorMap") {
    return std::make_unique<TakenFlagColorMap>();
  } else if (mappingName == "RLColorMap") {
    return std::make_unique<RLColorMap>();
  } else if (mappingName == "PosFlagColorMap") {
    return std::make_unique<PosFlagColorMap>();
  } else if (mappingName == "BackgroundTagColorMap") {
    return std::make_unique<BackgroundTagColorMap>();
  } else if (mappingName == "MCSegmentIdColorMap") {
    return std::make_unique<MCSegmentIdColorMap>();
  } else if (mappingName == "TOFColorMap") {
    return std::make_unique<TOFColorMap>();
  } else if (mappingName == "ReassignedSecondaryMap") {
    return std::make_unique<ReassignedSecondaryMap>();
  } else if (mappingName == "MCParticleColorMap") {
    return std::make_unique<MCParticleColorMap>();
  } else if (mappingName == "MCPDGCodeColorMap") {
    return std::make_unique<MCPDGCodeColorMap>();
  } else if (mappingName == "MCPrimaryColorMap") {
    return std::make_unique<MCPrimaryColorMap>();
  } else if (mappingName == "SimHitPDGCodeColorMap") {
    return std::make_unique<SimHitPDGCodeColorMap>();
  } else if (mappingName == "SimHitIsBkgColorMap") {
    return std::make_unique<SimHitIsBkgColorMap>();
  } else if (mappingName == "NLoops") {
    return std::make_unique<NLoopsColorMap>();
  } else {
    return Super::createMapping(mappingName);
  }
}
