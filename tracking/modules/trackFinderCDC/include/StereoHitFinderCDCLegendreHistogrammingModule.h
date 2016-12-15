#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/findlets/combined/StereoHitFinder.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
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
    class StereoHitFinderCDCLegendreHistogrammingModule : public FindletModule<StereoHitFinder> {

      typedef FindletModule<StereoHitFinder> Super;

    public:
      /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
      StereoHitFinderCDCLegendreHistogrammingModule();
    };
  }
}
