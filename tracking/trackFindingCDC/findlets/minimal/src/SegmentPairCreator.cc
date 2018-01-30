/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>
#include <array>
#include <string>
#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentPairCreator::SegmentPairCreator()
{
  this->addProcessingSignalListener(&m_segmentPairFilter);
  this->addProcessingSignalListener(&m_axialSegmentPairCreator);
}

std::string SegmentPairCreator::getDescription()
{
  return "Creates axial stereo segment pairs from a set of segments filtered by some acceptance criterion";
}

void SegmentPairCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_segmentPairFilter.exposeParameters(moduleParamList, prefix);
  m_axialSegmentPairCreator.exposeParameters(moduleParamList, prefixed("Axial", prefix));

  moduleParamList->addParameter(prefixed(prefix, "axialBridging"),
                                m_param_axialBridging,
                                "Switch to enable the search for axial to axial pairs "
                                "to enable more stable reconstruction of the middle stereo",
                                m_param_axialBridging);
}

void SegmentPairCreator::apply(const std::vector<CDCSegment2D>& inputSegments,
                               std::vector<CDCSegmentPair>& segmentPairs)
{
  segmentPairs.reserve(500);

  // Group the segments by their super layer id
  for (std::vector<const CDCSegment2D*>& segmentsInSuperLayer : m_segmentsBySuperLayer) {
    segmentsInSuperLayer.clear();
  }

  for (const CDCSegment2D& segment : inputSegments) {
    if (segment.empty()) continue;
    ISuperLayer iSuperLayer = segment.front().getISuperLayer();
    if (not ISuperLayerUtil::isInvalid(iSuperLayer)) {
      const CDCSegment2D* ptrSegment = &segment;
      m_segmentsBySuperLayer[iSuperLayer].push_back(ptrSegment);
    }
  }

  // Make pairs of closeby superlayers
  for (ISuperLayer iSuperLayer = 0; iSuperLayer < ISuperLayerUtil::c_N; ++iSuperLayer) {
    const std::vector<const CDCSegment2D*>& fromSegments = m_segmentsBySuperLayer[iSuperLayer];

    // Make pairs of this superlayer and the superlayer more to the inside
    ISuperLayer iSuperLayerIn = ISuperLayerUtil::getNextInwards(iSuperLayer);
    if (ISuperLayerUtil::isInCDC(iSuperLayerIn)) {
      const std::vector<const CDCSegment2D*>& toSegments = m_segmentsBySuperLayer[iSuperLayerIn];
      create(fromSegments, toSegments, segmentPairs);
    }

    // Make pairs of this superlayer and the superlayer more to the outside
    ISuperLayer iSuperLayerOut = ISuperLayerUtil::getNextOutwards(iSuperLayer);
    if (ISuperLayerUtil::isInCDC(iSuperLayerOut)) {
      const std::vector<const CDCSegment2D*>& toSegments = m_segmentsBySuperLayer[iSuperLayerOut];
      create(fromSegments, toSegments, segmentPairs);
    }
  }

  std::sort(segmentPairs.begin(), segmentPairs.end());

  if (not m_param_axialBridging) return;

  // Memory for the axial axial segment pairs.
  std::vector<CDCAxialSegmentPair> axialSegmentPairs;

  // Create the axial to axial bridges
  m_axialSegmentPairCreator.apply(inputSegments, axialSegmentPairs);

  // Make pairs by splitting the axial segment pairs and using the *common* fit as the 3d reconstruction bases
  for (const CDCAxialSegmentPair& axialSegmentPair : axialSegmentPairs) {
    const CDCSegment2D* startSegment = axialSegmentPair.getStartSegment();
    const CDCSegment2D* endSegment = axialSegmentPair.getEndSegment();

    CDCTrajectory2D startCommonTrajectory = axialSegmentPair.getTrajectory2D();
    if (not startCommonTrajectory.isFitted()) continue;

    CDCTrajectory2D startTrajectory2D = startSegment->getTrajectory2D();
    startCommonTrajectory.setLocalOrigin(startTrajectory2D.getLocalOrigin());
    startSegment->setTrajectory2D(startCommonTrajectory);
    std::vector<const CDCSegment2D*> startSegments{startSegment};

    CDCTrajectory2D endCommonTrajectory = axialSegmentPair.getTrajectory2D();
    CDCTrajectory2D endTrajectory2D = endSegment->getTrajectory2D();
    endCommonTrajectory.setLocalOrigin(endTrajectory2D.getLocalOrigin());
    endSegment->setTrajectory2D(endCommonTrajectory);
    std::vector<const CDCSegment2D*> endSegments{endSegment};

    if (startSegment->getISuperLayer() == endSegment->getISuperLayer()) {
      ISuperLayer iSuperLayerCommon = startSegment->getISuperLayer();

      ISuperLayer iSuperLayerIn = ISuperLayerUtil::getNextInwards(iSuperLayerCommon);
      if (ISuperLayerUtil::isInCDC(iSuperLayerIn)) {
        const std::vector<const CDCSegment2D*>& middleSegments = m_segmentsBySuperLayer[iSuperLayerIn];
        create(startSegments, middleSegments, segmentPairs);
        create(middleSegments, endSegments, segmentPairs);
      }

      ISuperLayer iSuperLayerOut = ISuperLayerUtil::getNextOutwards(iSuperLayerCommon);
      if (ISuperLayerUtil::isInCDC(iSuperLayerOut)) {
        const std::vector<const CDCSegment2D*>& middleSegments = m_segmentsBySuperLayer[iSuperLayerOut];
        create(startSegments, middleSegments, segmentPairs);
        create(middleSegments, endSegments, segmentPairs);
      }

    } else {
      ISuperLayer iSuperLayerMiddle =
        (startSegment->getISuperLayer() + endSegment->getISuperLayer()) / 2;
      const std::vector<const CDCSegment2D*>& middleSegments = m_segmentsBySuperLayer[iSuperLayerMiddle];
      create(startSegments, middleSegments, segmentPairs);
      create(middleSegments, endSegments, segmentPairs);
    }
    startSegment->setTrajectory2D(startTrajectory2D);
    endSegment->setTrajectory2D(endTrajectory2D);
  }

  auto lessPairAndgreaterWeight = [](const CDCSegmentPair & lhs, const CDCSegmentPair & rhs) {
    if (lhs < rhs) return true;
    if (rhs < lhs) return false;
    return lhs.getAutomatonCell().getCellWeight() > rhs.getAutomatonCell().getCellWeight();
  };

  std::sort(segmentPairs.begin(), segmentPairs.end(), lessPairAndgreaterWeight);

  // auto samePair = [](const CDCSegmentPair& lhs, const CDCSegmentPair& rhs) {
  //   if (lhs < rhs) return false;
  //   if (rhs < lhs) return false;
  //   return true;
  // };
  // erase_unique(segmentPairs, samePair);
}

void SegmentPairCreator::create(const std::vector<const CDCSegment2D*>& fromSegments,
                                const std::vector<const CDCSegment2D*>& toSegments,
                                std::vector<CDCSegmentPair>& segmentPairs)
{
  CDCSegmentPair segmentPair;
  for (const CDCSegment2D* ptrFromSegment : fromSegments) {
    for (const CDCSegment2D* ptrToSegment : toSegments) {

      if (ptrFromSegment == ptrToSegment) continue;
      segmentPair.setSegments(ptrFromSegment, ptrToSegment);
      segmentPair.clearTrajectory3D();

      Weight pairWeight = m_segmentPairFilter(segmentPair);
      if (not std::isnan(pairWeight)) {
        segmentPair.getAutomatonCell().setCellWeight(pairWeight);
        segmentPairs.push_back(segmentPair);
      }
    }
  }
}
