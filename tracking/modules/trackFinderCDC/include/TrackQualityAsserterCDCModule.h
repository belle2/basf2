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
   * Mainly the corrections are applied to remove fakes and to make the tracks fitable with genfit (which fails mainly for low-momentum tracks).
   * WARNING: Not all of the correction functions work well and the finding efficiency may be reduced strongly even when applying the correctly working
   * functions! Handle with care ;-)
   */
  class TrackQualityAsserterCDCModule: public TrackFinderCDCBaseModule {

  public:
    TrackQualityAsserterCDCModule() : TrackFinderCDCBaseModule()
    {
      setDescription("Many tracks in the CDC can not be fitted. For fitting them, we remove parts of the hits or maybe the whole track.");

      addParam("corrections", m_param_corrections,
               "The list of corrections to apply. Choose from LayerBreak, LargeAngle, LargeBreak2, OneSuperlayer, Small, B2B, MoveToNextAxial, None, Split, and ArcLength2D.",
      {std::string("LayerBreak"), std::string("LargeAngle"), std::string("OneSuperlayer"), std::string("Small")});

      addParam("onlyNotFittedTracks", m_param_onlyNotFittedTracks,
               "Flag to apply the corrections only to not fitted tracks.", false);
    }

  private:
    /**
     * Go through all tracks and correct them to be better fitable.
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// The corrections to use.
    std::vector<std::string> m_param_corrections;

    /// Flag to use the corrections only for not fitted tracks.
    bool m_param_onlyNotFittedTracks = false;
  };

}
