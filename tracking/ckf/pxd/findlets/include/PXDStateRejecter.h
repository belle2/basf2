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
#include <tracking/ckf/pxd/filters/states/ChooseableOnPXDStateApplier.h>

namespace Belle2 {
  class CKFToPXDState;

  /// Rejecter findlet for CKF PXD states
  extern template class StateRejecter<CKFToPXDState, ChooseableOnPXDStateApplier>;
  using PXDStateRejecter = StateRejecter<CKFToPXDState, ChooseableOnPXDStateApplier>;
}
