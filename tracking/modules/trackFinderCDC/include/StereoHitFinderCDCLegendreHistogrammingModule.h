#pragma once
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class StereohitsProcesser;
  }

  /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.
   *
   * A two-dimensional quad tree is used with $z_0$ in the one axis and the s-z-slope (tan lambda) on the other.
   * The module goes through each found axial track and for each track reconstructs either all stereo hits from the CDCWireHitTopology
   * or all found segments to match the r-phi trajectory of this track. The z-information is then used to determine the bin in the quad tree.
   * The hits resemble a straight line in the quad tree in z_0-tan lambda.
   *
   * The module can use the old and the new quad tree implementation - however the new one is prefered because it contains
   * a optimization step the old one does not have in the moment (but can be implemented easily).
   *
   * TODO:
   *   - Smarter Handling of segments (which is worse in the moment but should be better in general)
   *   - Make the precuts on the hits more transparent
   *  */
  class StereoHitFinderCDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  public:
    /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
    StereoHitFinderCDCLegendreHistogrammingModule();

    /** Initialize the stereo quad trees. */
    void initialize() override;

    /** Terminate the stereo quad trees. */
    void terminate() override;

  private:
    /**
     * Do a QuadTreeHistogramming with all the StereoHits.
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /// Parameters
    bool m_param_debugOutput; /**< Flag to turn on debug output. */
    unsigned int m_param_quadTreeLevel; /**< The number of levels for the quad tree search. */
    unsigned int m_param_minimumHitsInQuadTree; /**< The minimum number of hits in a quad tree bin to be called as result. */
    bool m_param_useOldImplementation; /**< Whether to use the old implementation o the quad tree. */
    bool m_param_useSegments; /**< Whether to use the segments or the hits in the quad tree. */
    bool m_param_checkForB2BTracks; /**< Set to false to skip the check for back-2-back tracks (good for cosmics). */

    TrackFindingCDC::StereohitsProcesser* m_stereohitsProcesser; /**< The stereo quad tree processor to use. */
  };

}
