#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class StereohitsProcesser;
  }

  /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
  class StereoHitFinderCDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  public:
    /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
    StereoHitFinderCDCLegendreHistogrammingModule();

    /** Initialize the stereo quad trees */
    void initialize() override;

    /** Terminate the stereo quad trees */
    void terminate() override;

  private:
    /**
     * Do a QuadTreeHistogramming with all the StereoHits
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// Parameters
    bool m_param_debugOutput; /**< Flag to turn on debug output */
    unsigned int m_param_quadTreeLevel; /**< The number of levels for the quad tree search */
    unsigned int m_param_minimumHitsInQuadTree; /**< The minimum number of hits in a quad tree bin to be called as result. */
    bool m_param_useOldImplementation; /**< Whether to use the old implementation o the quad tree. */
    bool m_param_checkForB2BTracks; /**< Set to false to skip the check for back-2-back tracks (good for cosmics) */

    TrackFindingCDC::StereohitsProcesser* m_stereohitsProcesser; /**< The stereo quad tree processor to use */
  };

}
