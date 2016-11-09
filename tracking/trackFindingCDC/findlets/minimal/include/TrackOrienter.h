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
      using Super = Findlet<const CDCTrack, CDCTrack>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;
      /// Signals the beginning of the event processing
      void initialize() override;

    public:
      /**
       *  Set the default output orientation of the tracks.
       *  * EPreferredDirection::c_None does not modify the orientation from the concret algorithm.
       *  * EPreferredDirection::c_Symmetric makes two copies of each track with forward and backward to the original orientation.
       *  * EPreferredDirection::c_Outwards flips the orientation of the track such that they point away from the interaction point.
       *  * EPreferredDirection::c_Downwards flips the orientation of the track such that they point downwards.
       *  * EPreferredDirection::c_Curling makes two copies for tracks that are likely curlers, fix others to outwards
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
                         std::vector<CDCTrack>& outputTracks) override final;

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
