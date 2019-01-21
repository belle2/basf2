/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/results/BasePXDResultFilter.h>
#include <tracking/ckf/pxd/filters/results/PXDResultFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  extern template class TrackFindingCDC::Chooseable<BasePXDResultFilter>;
  extern template class TrackFindingCDC::ChooseableFilter<PXDResultFilterFactory>;
  using ChooseablePXDResultFilter = TrackFindingCDC::ChooseableFilter<PXDResultFilterFactory>;
}