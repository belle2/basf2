/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/TrackQualityRejecter.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the MCTrackTrackQualityRejecter findlet
     */
    class TFCDC_TrackQualityRejecterModule : public FindletModule<TrackQualityRejecter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackQualityRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackQualityRejecterModule();
    };
  }
}
