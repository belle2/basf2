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
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }

  class TrackQualityAsserterCDCModule: public TrackFinderCDCBaseModule {

  public:
    TrackQualityAsserterCDCModule() : TrackFinderCDCBaseModule()
    {
      setDescription("Many tracks in the CDC can not be fitted. For fitting them, we remove parts of the hits or maybe the whole track.");

      addParam("corrections", m_param_corrections,
               "The list of corrections to apply. Choose from LayerBreak, LargeAngle, OneSuperlayer, Small.",
      {std::string("LayerBreak"), std::string("LargeAngle"), std::string("OneSuperlayer"), std::string("Small")});

      addParam("onlyNotFittedTracks", m_param_onlyNotFittedTracks,
               "Flag to use the corrections only for not fitted tracks", false);
    }

  private:
    /**
     * Go through all tracks and correct them to be better fitable.
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// The corrections to use
    std::vector<std::string> m_param_corrections;

    /// Flag to use the corrections only for not fitted tracks
    bool m_param_onlyNotFittedTracks = false;
  };

}
