/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackOrienter::getDescription()
{
  return "Fixes the flight direction of tracks to a preferred orientation by simple heuristics.";
}

void TrackOrienter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "TrackOrientation"),
                                m_param_trackOrientationString,
                                "Option which orientation of tracks shall be generate. "
                                "Valid options are '' (default of the finder), "
                                "'none' (one orientation, algorithm dependent), "
                                "'symmetric', "
                                "'curling', "
                                "'outwards', "
                                "'downwards'.",
                                m_param_trackOrientationString);
}

void TrackOrienter::initialize()
{
  Super::initialize();
  if (m_param_trackOrientationString != std::string("")) {
    try {
      m_trackOrientation = getPreferredDirection(m_param_trackOrientationString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected 'TrackOrientation' parameter : '" << m_param_trackOrientationString);
    }
  }
}

void TrackOrienter::apply(const std::vector<CDCTrack>& inputTracks,
                          std::vector<CDCTrack>& outputTracks)
{
  /// Copy tracks to output fixing their orientation
  if (m_trackOrientation == EPreferredDirection::c_None) {
    // Copy the tracks unchanged.
    outputTracks = inputTracks;

  } else if (m_trackOrientation == EPreferredDirection::c_Symmetric) {
    outputTracks.reserve(2 * inputTracks.size());
    for (const CDCTrack& track : inputTracks) {
      outputTracks.push_back(track.reversed());
      outputTracks.push_back(track);
    }

  } else if (m_trackOrientation == EPreferredDirection::c_Curling) {
    // Only make a copy for tracks that are curling inside the CDC
    // Others fix to flighing outwards
    outputTracks.reserve(1.5 * inputTracks.size());
    for (const CDCTrack& track : inputTracks) {
      const CDCTrajectory3D& startTrajectory3D = track.getStartTrajectory3D();
      const CDCTrajectory2D startTrajectory2D = startTrajectory3D.getTrajectory2D();

      const CDCTrajectory3D& endTrajectory3D = track.getEndTrajectory3D();
      const CDCTrajectory2D endTrajectory2D = endTrajectory3D.getTrajectory2D();
      bool isFitted = startTrajectory2D.isFitted() and endTrajectory2D.isFitted();
      bool isStartLeaver = (not endTrajectory2D.isCurler(1.1)) and startTrajectory2D.isOriginer();
      bool isEndLeaver = (not startTrajectory2D.isCurler(1.1)) and endTrajectory2D.isOriginer();
      // Trajectory is leaving the CDC starting in the inner volume
      bool isLeaver = isFitted and (isStartLeaver or isEndLeaver);
      if (isLeaver) {
        // Fix to outwards flying
        const CDCRecoHit3D& firstHit = track.front();
        const CDCRecoHit3D& lastHit = track.back();
        if (lastHit.getRecoPos2D().cylindricalR() < firstHit.getRecoPos2D().cylindricalR()) {
          outputTracks.push_back(track.reversed());
        } else {
          outputTracks.push_back(track);
        }
      } else {
        // Ambigious keep both options
        outputTracks.push_back(track);
        outputTracks.push_back(track.reversed());
      }
    }

  } else if (m_trackOrientation == EPreferredDirection::c_Outwards) {
    outputTracks.reserve(inputTracks.size());
    for (const CDCTrack& track : inputTracks) {
      const CDCRecoHit3D& firstHit = track.front();
      const CDCRecoHit3D& lastHit = track.back();
      if (lastHit.getRecoPos2D().cylindricalR() < firstHit.getRecoPos2D().cylindricalR()) {
        outputTracks.push_back(track.reversed());
      } else {
        outputTracks.push_back(track);
      }
    }

  } else if (m_trackOrientation == EPreferredDirection::c_Downwards) {
    outputTracks.reserve(inputTracks.size());
    for (const CDCTrack& track : inputTracks) {
      const CDCRecoHit3D& firstHit = track.front();
      const CDCRecoHit3D& lastHit = track.back();
      if (lastHit.getRecoPos2D().y() > firstHit.getRecoPos2D().y()) {
        outputTracks.push_back(track.reversed());
      } else {
        outputTracks.push_back(track);
      }
    }

  } else {
    B2WARNING("Unexpected 'TrackOrientation' parameter of track finder module : '" <<
              m_param_trackOrientationString <<
              "'. No tracks are put out.");
  }
}
