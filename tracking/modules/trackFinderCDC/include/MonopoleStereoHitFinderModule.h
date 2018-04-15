/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/MonopoleStereoHitFinder.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.
     *
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
