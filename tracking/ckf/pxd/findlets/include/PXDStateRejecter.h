/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/StateRejecter.dcl.h>
#include <tracking/ckf/pxd/filters/states/ChooseableOnPXDStateApplier.h>

namespace Belle2 {
  class CKFToPXDState;

  extern template class StateRejecter<CKFToPXDState, ChooseableOnPXDStateApplier>;
  /// Rejecter findlet for CKF PXD states
  using PXDStateRejecter = StateRejecter<CKFToPXDState, ChooseableOnPXDStateApplier>;
}
