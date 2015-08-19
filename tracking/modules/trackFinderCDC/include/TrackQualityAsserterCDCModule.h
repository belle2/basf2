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
    }

  private:
    /**
     * Go through all tracks and edit them to better fitable.
     * @param tracks
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /** Minimal Momentum under which to do the corrections. */
    double m_param_minimalMomentum = 0.4;
  };

}
