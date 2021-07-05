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

#include <tracking/ckf/cdc/filters/pathPairs/BaseCDCPathPairFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>


namespace Belle2 {
  /// For the two paths select the one with the most of MC matched hits.
  class MCTruthCDCPathPairFilter : public BaseCDCPathPairFilter {
  public:
    /// Input: pair of paths, returns 1 if pair.first to be selected, 0 otherwise.
    TrackFindingCDC::Weight operator()(const BaseCDCPathPairFilter::Object& pair) final;
  };
}
