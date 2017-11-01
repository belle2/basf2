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

#include <tracking/ckf/svd/filters/states/BaseSVDStateFilter.h>

namespace Belle2 {
  /// A very simple filter for all space points.
  class AllSVDStateFilter : public BaseSVDStateFilter {
  public:
    TrackFindingCDC::Weight operator()(const BaseSVDStateFilter::Object& pair) final;
  };
}
