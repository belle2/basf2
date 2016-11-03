/*
 * CDCSegmentColorMaps.cc
 *
 *  Created on: Jun 8, 2015
 *      Author: dschneider
 */

#include <tracking/trackFindingCDC/display/CDCSegmentColorMaps.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <sstream>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string inTrackIdToColor(const int inTrackId)
{
  double hue(50 * inTrackId % 360 / 360.);
  double saturation = 0.75;
  double lightness = 0.5;

  std::vector<double> rgb = HLSToRGB::hlsToRgb(hue, lightness, saturation);
  std::ostringstream oss;
  oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
  return oss.str();
}

std::string SegmentMCTrackIdColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  ITrackType mcTrackId = mcSegmentLookUp.getMCTrackId(&segment);
  if (mcTrackId < 0) {
    return m_bkgSegmentColor;
  } else {
    int m_iColor = mcTrackId % m_listColors.size();
    return m_listColors[m_iColor];
  }
}

std::string SegmentFBInfoColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  //Just to look at matched segments
  ITrackType mcTrackId = mcSegmentLookUp.getMCTrackId(&segment);
  if (mcTrackId < 0) return m_bkgSegmentColor;
  EForwardBackward fbInfo = mcSegmentLookUp.isForwardOrBackwardToMCTrack(&segment);
  if (fbInfo == EForwardBackward::c_Forward) {
    return "green";
  } else if (fbInfo == EForwardBackward::c_Backward) {
    return "red";
  } else {
    B2INFO("Segment not orientable to match track");
    return m_bkgSegmentColor;
  }
}

std::string SegmentFirstInTrackIdColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  //Just to look at matched segments
  Index firstInTrackId = mcSegmentLookUp.getFirstInTrackId(&segment);

  if (firstInTrackId < 0)return m_bkgSegmentColor;
  return inTrackIdToColor(firstInTrackId);
}

std::string SegmentLastInTrackIdColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  //Just to look at matched segments
  Index lastInTrackId = mcSegmentLookUp.getLastInTrackId(&segment);

  if (lastInTrackId < 0)return m_bkgSegmentColor;
  return inTrackIdToColor(lastInTrackId);
}

std::string SegmentFirstNPassedSuperLayersColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  //Just to look at matched segments
  Index firstNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(&segment);

  if (firstNPassedSuperLayers < 0)return m_bkgSegmentColor;
  return inTrackIdToColor(firstNPassedSuperLayers);
}

std::string SegmentLastNPassedSuperLayersColorMap::map(const int, const CDCRecoSegment2D& segment)
{
  CDCMCSegmentLookUp mcSegmentLookUp;
  mcSegmentLookUp.getInstance();

  //Just to look at matched segments
  Index lastNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(&segment);

  if (lastNPassedSuperLayers < 0)return m_bkgSegmentColor;
  return inTrackIdToColor(lastNPassedSuperLayers);
}

std::string SegmentListColorsColorMap::map(const int iSegment, const CDCRecoSegment2D&)
{
  return (m_listColors[iSegment % m_listColors.size()]);
}

