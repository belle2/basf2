/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/findlets/SVDStateRejecter.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>

namespace Belle2 {
  template class StateRejecter<CKFToSVDState, ChooseableOnSVDStateApplier>;
}
