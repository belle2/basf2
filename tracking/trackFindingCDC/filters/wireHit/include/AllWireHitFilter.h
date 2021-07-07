/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
