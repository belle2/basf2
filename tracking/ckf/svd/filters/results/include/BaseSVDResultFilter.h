/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

namespace Belle2 {
  /// Base filter for CKF SVD results (on overlap check)
  using BaseSVDResultFilter = TrackFindingCDC::Filter<CKFToSVDResult>;
}
