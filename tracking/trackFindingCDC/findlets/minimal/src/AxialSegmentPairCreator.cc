/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <vector>
#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialSegmentPairCreator::AxialSegmentPairCreator()
{
  this->addProcessingSignalListener(&m_axialSegmentPairFilter);
}

std::string AxialSegmentPairCreator::getDescription()
{
  return "Creates axial axial segment pairs from a set of segments filtered by some acceptance criterion";
}

void AxialSegmentPairCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_axialSegmentPairFilter.exposeParameters(moduleParamList, prefix);
}

void AxialSegmentPairCreator::apply(const std::vector<CDCSegment2D>& inputSegments,
                                    std::vector<CDCAxialSegmentPair>& axialSegmentPairs)
{
  // Group the segments by their super layer id
  for (std::vector<const CDCSegment2D*>& segementsInSuperLayer : m_segmentsBySuperLayer) {
    segementsInSuperLayer.clear();
  }

  for (const CDCSegment2D& segment : inputSegments) {
    ISuperLayer iSuperLayer = segment.getISuperLayer();
    const CDCSegment2D* ptrSegment = &segment;
    m_segmentsBySuperLayer[iSuperLayer].push_back(ptrSegment);
  }

  // Make pairs of closeby superlayers
  for (ISuperLayer iSuperLayer = 0; iSuperLayer < ISuperLayerUtil::c_N; ++iSuperLayer) {
    if (not ISuperLayerUtil::isAxial(iSuperLayer)) continue;

    const std::vector<const CDCSegment2D*>& startSegments = m_segmentsBySuperLayer[iSuperLayer];

    // Make pairs of this superlayer and the superlayer more to the inside
    {
      ISuperLayer iStereoSuperLayerIn = ISuperLayerUtil::getNextInwards(iSuperLayer);
      ISuperLayer iAxialSuperLayerIn = ISuperLayerUtil::getNextInwards(iStereoSuperLayerIn);
      if (ISuperLayerUtil::isInCDC(iAxialSuperLayerIn)) {
        const std::vector<const CDCSegment2D*>& endSegments = m_segmentsBySuperLayer[iAxialSuperLayerIn];
        create(startSegments, endSegments, axialSegmentPairs);
      }
    }

    // Make pairs with the same super layer
    create(startSegments, startSegments, axialSegmentPairs);

    // Make pairs of this superlayer and the superlayer more to the outside
    {
      ISuperLayer iStereoSuperLayerOut = ISuperLayerUtil::getNextOutwards(iSuperLayer);
      ISuperLayer iAxialSuperLayerOut = ISuperLayerUtil::getNextOutwards(iStereoSuperLayerOut);
      if (ISuperLayerUtil::isInCDC(iAxialSuperLayerOut)) {
        const std::vector<const CDCSegment2D*>& endSegments = m_segmentsBySuperLayer[iAxialSuperLayerOut];
        create(startSegments, endSegments, axialSegmentPairs);
      }
    }

  } // end for iSuperLayer

  std::sort(axialSegmentPairs.begin(), axialSegmentPairs.end());
}

void AxialSegmentPairCreator::create(const std::vector<const CDCSegment2D*>& startSegments,
                                     const std::vector<const CDCSegment2D*>& endSegments,
                                     std::vector<CDCAxialSegmentPair>& axialSegmentPairs)
{
  CDCAxialSegmentPair axialSegmentPair;
  for (const CDCSegment2D* ptrStartSegment : startSegments) {
    for (const CDCSegment2D* ptrEndSegment : endSegments) {

      if (ptrStartSegment == ptrEndSegment) continue; // Just for safety
      axialSegmentPair.setSegments(ptrStartSegment, ptrEndSegment);
      axialSegmentPair.clearTrajectory2D();

      Weight pairWeight = m_axialSegmentPairFilter(axialSegmentPair);
      if (not std::isnan(pairWeight)) {
        axialSegmentPair.getAutomatonCell().setCellWeight(pairWeight);
        axialSegmentPairs.push_back(axialSegmentPair);
      }
    }
  }
}
