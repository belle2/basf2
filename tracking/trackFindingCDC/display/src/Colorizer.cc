/*
 * Colorizer.cc
 *
 *  Created on: May 6, 2015
 *      Author: dschneider
 */
#include <tracking/trackFindingCDC/display/Colorizer.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCSegmentColorizer::CDCSegmentColorizer() : Colorizer(
{
  {"SegmentMCTrackIdColorMap", []() { return makeUnique<SegmentMCTrackIdColorMap>();}},
  {"SegmentFBInfoColorMap", []() { return makeUnique<SegmentFBInfoColorMap>();}},
  {"SegmentFirstInTrackIdColorMap", []() { return makeUnique<SegmentFirstInTrackIdColorMap>();}},
  {"SegmentLastInTrackIdColorMap", []() { return makeUnique<SegmentLastInTrackIdColorMap>();}},
  {"SegmentFirstNPassedSuperLayersColorMap", []() { return makeUnique<SegmentFirstNPassedSuperLayersColorMap>();}},
  {"SegmentLastNPassedSuperLayersColorMap", []() { return makeUnique<SegmentLastNPassedSuperLayersColorMap>();}},
  {"ListColors", []() { return makeUnique<SegmentListColorsColorMap>();}},
})
{
}

CDCHitColorizer::CDCHitColorizer() : Colorizer(
{
  {"ZeroDriftLengthColorMap", []() { return makeUnique<ZeroDriftLengthColorMap>();}},
  {"ZeroDriftLengthStrokeWidthMap", []() { return makeUnique<ZeroDriftLengthStrokeWidthMap>();}},
  {"TakenFlagColorMap", []() { return makeUnique<TakenFlagColorMap>();}},
  {"RLColorMap", []() { return makeUnique<RLColorMap>();}},
  {"PosFlagColorMap", []() { return makeUnique<PosFlagColorMap>();}},
  {"BackgroundTagColorMap", []() { return makeUnique<BackgroundTagColorMap>();}},
  {"MCSegmentIdColorMap", []() { return makeUnique<MCSegmentIdColorMap>();}},
  {"TOFColorMap", []() { return makeUnique<TOFColorMap>();}},
  {"ReassignedSecondaryMap", []() { return makeUnique<ReassignedSecondaryMap>();}},
  {"MCParticleColorMap", []() { return makeUnique<MCParticleColorMap>();}},
  {"MCPDGCodeColorMap", []() { return makeUnique<MCPDGCodeColorMap>();}},
  {"MCPrimaryColorMap", []() { return makeUnique<MCPrimaryColorMap>();}},
  {"SimHitPDGCodeColorMap", []() { return makeUnique<SimHitPDGCodeColorMap>();}},
  {"SimHitIsBkgColorMap", []() { return makeUnique<SimHitIsBkgColorMap>();}}
})
{
}
