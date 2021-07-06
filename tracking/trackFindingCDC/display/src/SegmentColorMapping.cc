/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/SegmentColorMapping.h>

#include <tracking/trackFindingCDC/display/Colors.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const std::string c_bkgSegmentColor = "orange";

namespace {
  std::string inTrackIdToColor(int inTrackId)
  {
    return Colors::getWheelColor(50 * inTrackId);
  }
}

SegmentMCTrackIdColorMap::SegmentMCTrackIdColorMap()
  : m_colors(Colors::getList())
{
}

std::string SegmentMCTrackIdColorMap::map(int index __attribute__((unused)),
                                          const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  ITrackType mcTrackId = mcSegmentLookUp.getMCTrackId(&segment);

  if (mcTrackId < 0) {
    return c_bkgSegmentColor;
  } else {
    int m_iColor = mcTrackId % m_colors.size();
    return m_colors[m_iColor];
  }
}

std::string SegmentFBInfoColorMap::map(int index __attribute__((unused)),
                                       const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
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
                                               const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  Index firstInTrackId = mcSegmentLookUp.getFirstInTrackId(&segment);

  if (firstInTrackId < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(firstInTrackId);
}

std::string SegmentLastInTrackIdColorMap::map(int index __attribute__((unused)),
                                              const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  Index lastInTrackId = mcSegmentLookUp.getLastInTrackId(&segment);

  if (lastInTrackId < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(lastInTrackId);
}

std::string SegmentFirstNPassedSuperLayersColorMap::map(int index __attribute__((unused)),
                                                        const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  Index firstNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(&segment);

  if (firstNPassedSuperLayers < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(firstNPassedSuperLayers);
}

std::string SegmentLastNPassedSuperLayersColorMap::map(int index __attribute__((unused)),
                                                       const CDCSegment2D& segment)
{
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  Index lastNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(&segment);

  if (lastNPassedSuperLayers < 0) return c_bkgSegmentColor;
  return inTrackIdToColor(lastNPassedSuperLayers);
}
