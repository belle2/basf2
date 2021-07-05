/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /** Findlet for inspecting and printing out CDCtracks on a R-z plane for debug purposes
     *
     * can be done before interfacing to genfit, while the track is just a vector of found hits
     */
    class TrackInspector : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Access parameters
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Print the tracks.
      void apply(std::vector<CDCTrack>& tracks) final;

      /// Remove tracks with no stereo hits
      void removeIncompleteTracks(std::vector<CDCTrack>& tracks);

    private:
      /// Flag to draw the CDCTrack (true) or not (false)
      bool m_param_debugDraw = false;
    };
  }
}

