/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Fixes the orientation of tracks by a simple heuristic
    class TrackOrienter:
      public Findlet<const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      typedef Findlet<const CDCTrack, CDCTrack> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Fixes the flight direction of tracks to a preferred orientation by simple heuristics.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        moduleParamList->addParameter(prefixed(prefix, "TrackOrientation"),
                                      m_param_trackOrientationString,
                                      "Option which orientation of tracks shall be generate. "
                                      "Valid options are '' (default of the finder), "
                                      "'none' (one orientation, algorithm dependent), "
                                      "'symmetric', "
                                      "'outwards', "
                                      "'downwards'.",
                                      std::string(m_param_trackOrientationString));
      }

      /// Signals the beginning of the event processing
      void initialize() override
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

    public:
      /** Set the default output orientation of the tracks.
       *  * EPreferredDirection::c_None does not modify the orientation from the concret algorithm.
       *  * EPreferredDirection::c_Symmetric makes two copies of each track with forward and backward to the original orientation.
       *  * EPreferredDirection::c_Outwards flips the orientation of the track such that they point away from the interaction point.
       *  * EPreferredDirection::c_Downwards flips the orientation of the track such that they point downwards.
       *
       *  This properties can also be overridden by the user by a module parameter.
       */
      void setTrackOrientation(const EPreferredDirection& trackOrientation)
      { m_trackOrientation = trackOrientation; }

      /// Get the currentl default output orientation of the tracks.
      const EPreferredDirection& getTrackOrientation() const
      { return m_trackOrientation; }

    public:
      /// Main algorithm applying the adjustment of the orientation.
      virtual void apply(const std::vector<CDCTrack>& inputTracks,
                         std::vector<CDCTrack>& outputTracks) override final
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

    private:
      /** Parameter: String that states the desired track orientation.
       *  Valid orientations are "none" (unchanged), "symmetric", "outwards", "downwards".
       */
      std::string m_param_trackOrientationString = "";

      /** Encoded desired track orientation.
       *  Valid orientations are "c_None" (unchanged), "c_Symmetric", "c_Outwards", "c_Downwards.
       */
      EPreferredDirection m_trackOrientation = EPreferredDirection::c_None;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
