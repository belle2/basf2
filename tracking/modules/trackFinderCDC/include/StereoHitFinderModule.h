/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/StereoHitFinder.h>
#include <tracking/trackFindingCDC/findlets/combined/MonopoleStereoHitFinder.h>
#include <tracking/trackFindingCDC/findlets/combined/MonopoleStereoHitFinderQuadratic.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
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
    class TFCDC_StereoHitFinderModule : public FindletModule<StereoHitFinder> {

      /// The base class
      using Super = FindletModule<StereoHitFinder>;

    public:
      /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a 2D hough quad tree. */
      TFCDC_StereoHitFinderModule();
    };

    /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.
     * WARNING This findlet is kept here just in case hyperbolic one misbehaves and eats up too much RAM
     * If it doesn't, this one should be removed around release-05 or earlier
     *
     * Same as above StereoHitFinder, but the s-z hypothesis is z(s) = (0) + (p + 4q)*x - q/25*s^2, where
     *  p (in units of 100cm) is z coordinate of track at s=100cm, i.e. outer layers of CDC
     *  q (in units of 100cm) is divergence of from a straight line at s=50cm, i.e. middle layers of CDC
     *
     * Non-zero q's indicate magnetic charge of the track
     *  */
    class TFCDC_MonopoleStereoHitFinderQuadraticModule : public FindletModule<MonopoleStereoHitFinderQuadratic> {

      /// The base class
      using Super = FindletModule<MonopoleStereoHitFinderQuadratic>;

    public:
      /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
      TFCDC_MonopoleStereoHitFinderQuadraticModule();
    };

    /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a 3D hough tree looking for hyperbolic cosines.
     *
     * Description of the actual algorithm is in tracking/trackFindingCDC/hough/algorithms/include/HitInHyperBox.h
     *  */
    class TFCDC_MonopoleStereoHitFinderModule : public FindletModule<MonopoleStereoHitFinder> {

      /// The base class
      using Super = FindletModule<MonopoleStereoHitFinder>;

    public:
      /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree. */
      TFCDC_MonopoleStereoHitFinderModule();
    };
  }
}
