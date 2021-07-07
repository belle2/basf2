/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <vector>

namespace Belle2 {
  /// Shortcut for the collection of CDC CKF-algorithm states
  using CDCCKFPath = std::vector<CDCCKFState>;

  /// Output operator for the collection of CDC CKF-algorithm states
  std::ostream& operator<<(std::ostream& output, const CDCCKFPath& path);
}
