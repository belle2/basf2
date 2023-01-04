/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/dbobjects/BaseTrackingMVAFilterPayload.h>

namespace Belle2 {
  /** Class for the MVA filter payloads. */
  class TrackQualityEstimatorPayload : public BaseTrackingMVAFilterPayload {
  public:
    /** Default constructor */
    TrackQualityEstimatorPayload() {}

    ClassDef(TrackQualityEstimatorPayload, 1);  /**< ClassDef, necessary for ROOT */
  };
}