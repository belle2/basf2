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

      addParam("minimalMomentum", m_param_minimalMomentum,
               "The minimal momentum under which the corrections take place. Use an arbitrary high value (like 10) to use all tracks.", 10.0);
      addParam("corrections", m_param_corrections,
               "The list of corrections to apply. Choose from LayerBreak, LargeAngle, OneSuperlayer, Small.",
      {std::string("LayerBreak"), std::string("LargeAngle"), std::string("OneSuperlayer"), std::string("Small")});
    }

  private:
    /**
     * Go through all tracks and edit them to better fitable.
     * @param tracks
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /** Minimal Momentum under which to do the corrections. */
    double m_param_minimalMomentum = 0.4;

    /** The corrections to use */
    std::vector<std::string> m_param_corrections;
  };

}
