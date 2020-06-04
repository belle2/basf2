/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {

  /// Give a weight based on the mc truth information (1 or NAN)
  class MCTruthEclSeedFilter : public BaseCDCStateFilter {
  public:
    /// return 1 if matched truth hit belongs to matched truth track, NAN otherwise
    TrackFindingCDC::Weight operator()(const BaseCDCStateFilter::Object& pair) final;
  };
}
