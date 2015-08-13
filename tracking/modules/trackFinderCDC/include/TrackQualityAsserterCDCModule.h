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
      setDescription("Many tracks in the cdc can not be fitted. For fitting them, we remove parts of the hits and maybe reverse them.");
      addParam("MinimalPerpSCut", m_param_minimalPerpSCut, "The cut to the perpS distances above a track should be cut.", 0.0);
    }

  private:
    /**
     * Go through all tracks and edit them to better fitable.
     * @param tracks
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    void terminate()
    {
      B2INFO("Having deleted " << m_numberOfDeletedHits << " hits of " << m_numberOfHits << " hits in total.")
      TrackFinderCDCBaseModule::terminate();
    }

    double m_param_minimalPerpSCut; /**< The cut to the perpS distances above a track should be cut. */

    /** Number of deleted hits */
    unsigned int m_numberOfDeletedHits = 0;
    /** Number of total hits */
    unsigned int m_numberOfHits = 0;
  };

}
