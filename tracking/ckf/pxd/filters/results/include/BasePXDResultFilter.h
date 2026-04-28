/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/Filter.icc.h>
#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>

namespace Belle2 {
  /// Base filter for CKF PXD results (on overlap check)
  using BasePXDResultFilter = TrackingUtilities::Filter<CKFToPXDResult>;
}
