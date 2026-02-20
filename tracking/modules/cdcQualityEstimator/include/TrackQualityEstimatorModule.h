/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/TrackQualityEstimator.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>
#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackTrackQualityEstimator findlet
     */
    class TFCDC_TrackQualityEstimatorModule : public TrackingUtilities::FindletModule<TrackQualityEstimator> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackQualityEstimator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackQualityEstimatorModule();
    };
  }
}
