/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/states/BaseSVDStateFilter.h>

namespace Belle2 {
  /// A very simple filter for all space points.
  class AllSVDStateFilter : public BaseSVDStateFilter {
  public:
    TrackFindingCDC::Weight operator()(const BaseSVDStateFilter::Object& pair) override;
  };
}
