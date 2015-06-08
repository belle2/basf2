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
  {"SegmentMCTrackIdColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentMCTrackIdColorMap>},
  {"SegmentFBInfoColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentFBInfoColorMap>},
  {"SegmentFirstInTrackIdColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentFirstInTrackIdColorMap>},
  {"SegmentLastInTrackIdColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentLastInTrackIdColorMap>},
  {"SegmentFirstNPassedSuperLayersColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentFirstNPassedSuperLayersColorMap>},
  {"SegmentLastNPassedSuperLayersColorMap", &CDCSegmentColorizer::constructMappingClass<SegmentLastNPassedSuperLayersColorMap>},
  {"ListColors", &CDCSegmentColorizer::constructMappingClass<SegmentListColorsColorMap>},
})
{
}

CDCHitColorizer::CDCHitColorizer() : Colorizer(
{
  {"ZeroDriftLengthColorMap", &CDCHitColorizer::constructMappingClass<ZeroDriftLengthColorMap>},
  {"ZeroDriftLengthStrokeWidthMap", &CDCHitColorizer::constructMappingClass<ZeroDriftLengthStrokeWidthMap>},
  {"TakenFlagColorMap", &CDCHitColorizer::constructMappingClass<TakenFlagColorMap>},
  {"RLColorMap", &CDCHitColorizer::constructMappingClass<RLColorMap>},
  {"PosFlagColorMap", &CDCHitColorizer::constructMappingClass<PosFlagColorMap>},
  {"BackgroundTagColorMap", &CDCHitColorizer::constructMappingClass<BackgroundTagColorMap>},
  {"MCSegmentIdColorMap", &CDCHitColorizer::constructMappingClass<MCSegmentIdColorMap>},
  {"TOFColorMap", &CDCHitColorizer::constructMappingClass<TOFColorMap>},
  {"ReassignedSecondaryMap", &CDCHitColorizer::constructMappingClass<ReassignedSecondaryMap>},
  {"MCParticleColorMap", &CDCHitColorizer::constructMappingClass<MCParticleColorMap>},
  {"MCPDGCodeColorMap", &CDCHitColorizer::constructMappingClass<MCPDGCodeColorMap>},
  {"MCPrimaryColorMap", &CDCHitColorizer::constructMappingClass<MCPrimaryColorMap>},
  {"SimHitPDGCodeColorMap", &CDCHitColorizer::constructMappingClass<SimHitPDGCodeColorMap>},
  {"SimHitIsBkgColorMap", &CDCHitColorizer::constructMappingClass<SimHitIsBkgColorMap>}
})
{
}
