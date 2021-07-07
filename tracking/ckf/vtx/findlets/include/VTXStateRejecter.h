/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/StateRejecter.dcl.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <tracking/ckf/vtx/filters/states/ChooseableOnVTXStateApplier.h>

namespace Belle2 {
  // TODO: this is the point where one wants to implement the cached advance filter!
  extern template class StateRejecter<CKFToVTXState, ChooseableOnVTXStateApplier>;

  /// Rejecter findlet for CKF VTX states
  using VTXStateRejecter = StateRejecter<CKFToVTXState, ChooseableOnVTXStateApplier>;
}
