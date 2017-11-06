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

#include <tracking/ckf/general/findlets/StateRejecter.dcl.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/ckf/svd/filters/states/ChooseableOnSVDStateApplier.h>

namespace Belle2 {
  /// Rejecter findlet for CKF SVD states
  // TODO: this is the point where one wants to implement the cached advance filter!
  using SVDStateRejecter = StateRejecter<CKFToSVDState, ChooseableOnSVDStateApplier>;
}
