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

using namespace Belle2;
using namespace TrackFindingCDC;

std::unique_ptr<ChooseableSegmentStyling::ObjectMapping>
ChooseableSegmentStyling::createMapping(const std::string& mappingName)
{
  if (mappingName == "SegmentMCTrackIdColorMap") {
    return makeUnique<SegmentMCTrackIdColorMap>();
  } else if (mappingName == "SegmentFBInfoColorMap") {
    return makeUnique<SegmentFBInfoColorMap>();
  } else if (mappingName == "SegmentFirstInTrackIdColorMap") {
    return makeUnique<SegmentFirstInTrackIdColorMap>();
  } else if (mappingName == "SegmentLastInTrackIdColorMap") {
    return makeUnique<SegmentLastInTrackIdColorMap>();
  } else if (mappingName == "SegmentFirstNPassedSuperLayersColorMap") {
    return makeUnique<SegmentFirstNPassedSuperLayersColorMap>();
  } else if (mappingName == "SegmentLastNPassedSuperLayersColorMap") {
    return makeUnique<SegmentLastNPassedSuperLayersColorMap>();
  } else if (mappingName == "ListColors") {
    return makeUnique<SegmentListColorsColorMap>();
  } else {
    return Super::createMapping(mappingName);
  }
}

std::unique_ptr<ChooseableHitStyling::ObjectMapping>
ChooseableHitStyling::createMapping(const std::string& mappingName)
{
  if (mappingName == "ZeroDriftLengthColorMap") {
    return makeUnique<ZeroDriftLengthColorMap>();
  } else if (mappingName == "ZeroDriftLengthStrokeWidthMap") {
    return makeUnique<ZeroDriftLengthStrokeWidthMap>();
  } else if (mappingName == "TakenFlagColorMap") {
    return makeUnique<TakenFlagColorMap>();
  } else if (mappingName == "RLColorMap") {
    return makeUnique<RLColorMap>();
  } else if (mappingName == "PosFlagColorMap") {
    return makeUnique<PosFlagColorMap>();
  } else if (mappingName == "BackgroundTagColorMap") {
    return makeUnique<BackgroundTagColorMap>();
  } else if (mappingName == "MCSegmentIdColorMap") {
    return makeUnique<MCSegmentIdColorMap>();
  } else if (mappingName == "TOFColorMap") {
    return makeUnique<TOFColorMap>();
  } else if (mappingName == "ReassignedSecondaryMap") {
    return makeUnique<ReassignedSecondaryMap>();
  } else if (mappingName == "MCParticleColorMap") {
    return makeUnique<MCParticleColorMap>();
  } else if (mappingName == "MCPDGCodeColorMap") {
    return makeUnique<MCPDGCodeColorMap>();
  } else if (mappingName == "MCPrimaryColorMap") {
    return makeUnique<MCPrimaryColorMap>();
  } else if (mappingName == "SimHitPDGCodeColorMap") {
    return makeUnique<SimHitPDGCodeColorMap>();
  } else if (mappingName == "SimHitIsBkgColorMap") {
    return makeUnique<SimHitIsBkgColorMap>();
  } else {
    return Super::createMapping(mappingName);
  }
}
