/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/SegmentColorMapping.h>

#include <tracking/trackFindingCDC/display/Colors.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <sstream>

using namespace Belle2;
using namespace TrackFindingCDC;

const std::string c_bkgSegmentColor = "orange";

namespace {
  std::string inTrackIdToColor(int inTrackId)
  {
    double hue(50 * inTrackId % 360 / 360.);
    double saturation = 0.75;
    double lightness = 0.5;

    std::array<double, 3> rgb = Colors::hlsToRgb(hue, lightness, saturation);
    std::ostringstream oss;
    oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
    return oss.str();
  }
}

SegmentMCTrackIdColorMap::SegmentMCTrackIdColorMap()
  : m_colors(Colors::getList())
{
}

std::string SegmentMCTrackIdColorMap::map(int index __attribute__((unused)),
                                          const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  ITrackType mcTrackId = mcSegmentLookUp.getMCTrackId(&segment);

  if (mcTrackId < 0) {
    return c_bkgSegmentColor;
  } else {
    int m_iColor = mcTrackId % m_colors.size();
    return m_colors[m_iColor];
  }
}

std::string SegmentFBInfoColorMap::map(int index __attribute__((unused)),
                                       const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  ITrackType mcTrackId = mcSegmentLookUp.getMCTrackId(&segment);

  if (mcTrackId < 0) return c_bkgSegmentColor;
  EForwardBackward fbInfo = mcSegmentLookUp.isForwardOrBackwardToMCTrack(&segment);
  if (fbInfo == EForwardBackward::c_Forward) {
    return "green";
  } else if (fbInfo == EForwardBackward::c_Backward) {
    return "red";
  } else {
    B2INFO("Segment not orientable to match track");
    return c_bkgSegmentColor;
  }
}

std::string SegmentFirstInTrackIdColorMap::map(int index __attribute__((unused)),
                                               const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  Index firstInTrackId = mcSegmentLookUp.getFirstInTrackId(&segment);

  if (firstInTrackId < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(firstInTrackId);
}

std::string SegmentLastInTrackIdColorMap::map(int index __attribute__((unused)),
                                              const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  Index lastInTrackId = mcSegmentLookUp.getLastInTrackId(&segment);

  if (lastInTrackId < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(lastInTrackId);
}

std::string SegmentFirstNPassedSuperLayersColorMap::map(int index __attribute__((unused)),
                                                        const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  Index firstNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(&segment);

  if (firstNPassedSuperLayers < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(firstNPassedSuperLayers);
}

std::string SegmentLastNPassedSuperLayersColorMap::map(int index __attribute__((unused)),
                                                       const CDCRecoSegment2D& segment)
{
  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  Index lastNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(&segment);

  if (lastNPassedSuperLayers < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(lastNPassedSuperLayers);
}
