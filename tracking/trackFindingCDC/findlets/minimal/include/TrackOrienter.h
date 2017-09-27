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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Fixes the orientation of tracks by a simple heuristic
    class TrackOrienter : public Findlet<const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCTrack, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signals the beginning of the event processing
      void initialize() final;

      /// Main algorithm applying the adjustment of the orientation.
      void apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks) final;

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
