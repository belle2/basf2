/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {

    /// Fixes the orientation of tracks by a simple heuristic
    class TrackOrienter : public TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signals the beginning of the event processing
      void initialize() final;

      /// Main algorithm applying the adjustment of the orientation.
      void apply(const std::vector<TrackingUtilities::CDCTrack>& inputTracks,
                 std::vector<TrackingUtilities::CDCTrack>& outputTracks) final;

    private:
      /**
       *  Parameter: String that states the desired track orientation.
       *  Valid orientations are "none" (unchanged), "outwards", "downwards", "symmetric", "curling"
       */
      std::string m_param_trackOrientationString = "";

      /**
       *  Encoded desired track orientation.
       *  Valid orientations are "c_None" (unchanged), "c_Outwards", "c_Downwards", "c_Symmetric", "c_Curling",
       */
      EPreferredDirection m_trackOrientation = EPreferredDirection::c_None;
    };
  }
}
