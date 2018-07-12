/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentOrienter::getDescription()
{
  return "Fixes the flight direction of segments to a preferred orientation by simple heuristics.";
}


void SegmentOrienter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "SegmentOrientation"),
                                m_param_segmentOrientationString,
                                "Option which orientation of segments shall be generate. "
                                "Valid options are '' (default of the finder), "
                                "'none' (one orientation, algorithm dependent), "
                                "'symmetric', "
                                "'curling', "
                                "'outwards', "
                                "'downwards'.",
                                m_param_segmentOrientationString);
}

void SegmentOrienter::initialize()
{
  Super::initialize();
  if (m_param_segmentOrientationString != std::string("")) {
    try {
      m_segmentOrientation = getPreferredDirection(m_param_segmentOrientationString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected 'SegmentOrientation' parameter : '" << m_param_segmentOrientationString);
    }
  }
}

void SegmentOrienter::apply(const std::vector<CDCSegment2D>& inputSegments,
                            std::vector<CDCSegment2D>& outputSegments)
{
  /// Copy segments to output fixing their orientation
  if (m_segmentOrientation == EPreferredDirection::c_None) {
    // Copy the segments unchanged.
    outputSegments = inputSegments;

  } else if (m_segmentOrientation == EPreferredDirection::c_Symmetric) {
    outputSegments.reserve(2 * inputSegments.size());
    for (const CDCSegment2D& segment : inputSegments) {
      outputSegments.push_back(segment);
      if (segment->hasReverseFlag()) continue; // Already a reverse found in the facet ca
      outputSegments.back()->setReverseFlag();
      outputSegments.push_back(segment.reversed());
      outputSegments.back()->setReverseFlag();
    }

  } else if (m_segmentOrientation == EPreferredDirection::c_Curling) {
    // Only make a copy for segments that are curling inside the CDC
    // Others fix to flighing outwards
    outputSegments.reserve(1.5 * inputSegments.size());
    for (const CDCSegment2D& segment : inputSegments) {
      if (segment->hasReverseFlag()) {
        outputSegments.push_back(segment);
        continue; // Already a reverse found in the facet ca
      }
      const CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
      bool isFitted = trajectory2D.isFitted();
      bool isCurler = trajectory2D.isCurler(1.1);
      bool isOriginer = trajectory2D.isOriginer();
      // Trajectory is leaving the CDC starting in the inner volume
      bool isLeaver = isFitted and (not isCurler) and isOriginer;
      if (isLeaver) {
        // Fix to outwards flying
        const CDCRecoHit2D& firstHit = segment.front();
        const CDCRecoHit2D& lastHit = segment.back();
        if (lastHit.getRecoPos2D().cylindricalR() < firstHit.getRecoPos2D().cylindricalR()) {
          outputSegments.push_back(segment.reversed());
        } else {
          outputSegments.push_back(segment);
        }
      } else {
        // Ambigious keep both options
        outputSegments.push_back(segment);
        outputSegments.back()->setReverseFlag();
        outputSegments.push_back(segment.reversed());
        outputSegments.back()->setReverseFlag();
      }
    }

  } else if (m_segmentOrientation == EPreferredDirection::c_Outwards) {
    outputSegments.reserve(inputSegments.size());
    for (const CDCSegment2D& segment : inputSegments) {
      const CDCRecoHit2D& firstHit = segment.front();
      const CDCRecoHit2D& lastHit = segment.back();
      if (lastHit.getRecoPos2D().cylindricalR() < firstHit.getRecoPos2D().cylindricalR()) {
        outputSegments.push_back(segment.reversed());
      } else {
        outputSegments.push_back(segment);
      }
    }
  } else if (m_segmentOrientation == EPreferredDirection::c_Downwards) {
    outputSegments.reserve(inputSegments.size());
    for (const CDCSegment2D& segment : inputSegments) {
      const CDCRecoHit2D& firstHit = segment.front();
      const CDCRecoHit2D& lastHit = segment.back();
      if (lastHit.getRecoPos2D().y() > firstHit.getRecoPos2D().y()) {
        outputSegments.push_back(segment.reversed());
      } else {
        outputSegments.push_back(segment);
      }
    }

  } else {
    B2WARNING("Unexpected 'SegmentOrientation' parameter of segment finder module : '" <<
              m_param_segmentOrientationString <<
              "'. No segments are put out.");
  }
}
