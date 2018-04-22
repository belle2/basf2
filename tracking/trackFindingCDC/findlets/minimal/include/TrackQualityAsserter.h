/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <string>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;

    /**
     * This module applies configurable correction functions to all found tracks.
     *
     * Typical correction functions include:
     *   - Removal of hits after long breaks
     *   - Removal of hits on the wrong side of the detector
     *   - Splitting of curling tracks into two halves
     *   - etc.
     *
     * See the TrackQualityTools for details on the specific functions.
     *
     * Mainly the corrections are applied to remove fakes and to make the tracks fitable with genfit
     * (which fails mainly for low-momentum tracks).
     * WARNING: Not all of the correction functions work well and the finding efficiency may be
     * reduced strongly even when applying the correctly working
     * functions! Handle with care ;-)
     */
    class TrackQualityAsserter: public Findlet<CDCTrack&> {

      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Constructor setting up the default parameters
      TrackQualityAsserter();

      /// Get the description of the findlet
      std::string getDescription() override;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function to clean up the tracks
      void apply(std::vector<CDCTrack>& tracks) final;

    private:
      /// Parameter : The corrections to use.
      std::vector<std::string> m_param_corrections;

      /// Parameter : Flag to use the corrections only for not fitted tracks.
      bool m_param_onlyNotFittedTracks = false;
    };
  }
}
