/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Filter accepting all hits
    class AllWireHitFilter : public BaseWireHitFilter {

    public:
      /// Basic filter method to override. All implementation accepts all hits.
      Weight operator()(const CDCWireHit& wireHit) final;
    };
  }
}
