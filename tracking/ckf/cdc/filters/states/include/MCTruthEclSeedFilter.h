/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
