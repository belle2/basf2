/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/TrackQualityEstimator.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackTrackQualityEstimator findlet
     */
    class TFCDC_TrackQualityEstimatorModule : public FindletModule<TrackQualityEstimator> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackQualityEstimator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackQualityEstimatorModule();
    };
  }
}
