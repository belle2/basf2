#pragma once

#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitCollector.h>
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
  }

  /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.
   *
   * A two-dimensional quad tree is used with $z_0$ in the one axis and the s-z-slope (tan lambda) on the other.
   * The module goes through each found axial track and for each track reconstructs either all stereo hits from the CDCWireHitVector
   * or all found segments to match the r-phi trajectory of this track. The z-information is then used to determine the bin in the quad tree.
   * The hits resemble a straight line in the quad tree in z_0-tan lambda.
   *
   * The module can use the old and the new quad tree implementation - however the new one is prefered because it contains
   * a optimization step the old one does not have in the moment (but can be implemented easily).
   *
   * Everything action in hidden in the configurable collector algorithm, which can be used to perform various
   * collection algorithms, e.g. using the first found or best hit, using various filters/TMVA/etc.
   * The options can be configured using the module parameters.
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
    void generate(std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    /// Collector hiding the implementation.
    TrackFindingCDC::StereoHitCollectorSingleMatch m_stereohitsCollectorSingleMatch;

    /// Collector hiding the implementation.
    TrackFindingCDC::StereoHitCollectorFirstMatch m_stereohitsCollectorFirstMatch;

    /// Parameter
    /// If true, use the single match instead of the first match algorithm.
    bool m_param_useSingleMatchAlgorithm = false;
  };

}
