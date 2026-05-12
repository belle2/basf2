/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/FindletModule.h>
#include <tracking/vxdHoughTracking/findlets/SVDHoughTracking.h>

namespace Belle2 {
  /**
   * Full Hough Transformation based SVD track finding.
   */
  class SVDHoughTrackingModule : public TrackingUtilities::FindletModule<vxdHoughTracking::SVDHoughTracking> {

  public:
    /// Set description
    SVDHoughTrackingModule()
    {
      setDescription("Full Hough Transformation based SVD track finding.");
    }
  };

}
