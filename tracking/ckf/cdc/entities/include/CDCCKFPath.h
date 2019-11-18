/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
