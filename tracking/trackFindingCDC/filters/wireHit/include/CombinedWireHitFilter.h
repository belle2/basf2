/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/CutsFromDBWireHitFilter.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCWireHit;
  }
  namespace TrackFindingCDC {

    /// Filter rejecting hits according to DB values.
    class CombinedWireHitFilter : public BaseWireHitFilter {

    public:
      /// Default constructor.
      CombinedWireHitFilter();

      /// Default destructor.
      virtual ~CombinedWireHitFilter() = default;

      /// Called at the beginning of the processing.
      void initialize() final;

      /// Called when a new run is started.
      void beginRun() final;

      /// Basic filter method to override.
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCWireHit& wireHit) final;

    private:
      /// MVA filter

      /// Cuts from DB filter
      CutsFromDBWireHitFilter m_cutsFromDBFilter;
    };
  }
}
