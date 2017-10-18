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

#include <tracking/ckf/general/findlets/LimitedOnStateApplier.dcl.h>
#include <tracking/ckf/pxd/filters/states/PXDStateFilterFactory.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  using ChooseableOnPXDStateApplier = LimitedOnStateApplier<CKFToPXDState,
        TrackFindingCDC::ChooseableFilter<PXDStateFilterFactory>>;
}