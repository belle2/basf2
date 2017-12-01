/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/findlets/PXDStateRejecter.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

using namespace Belle2;

template class Belle2::StateRejecter<CKFToPXDState, ChooseableOnPXDStateApplier>;