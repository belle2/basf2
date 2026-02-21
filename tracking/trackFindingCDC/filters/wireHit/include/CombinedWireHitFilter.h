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
#include <tracking/trackingUtilities/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/wireHit/CDCWireHitVarSet.h>
#include <framework/database/DBObjPtr.h>
#include <tracking/dbobjects/WireHitFilterSettings.h>

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

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Basic filter method to override.
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCWireHit& wireHit) final;

    private:
      /// MVA filter
      TrackingUtilities::MVAFilter<CDCWireHitVarSet> m_mvaFilter;

      /// Cuts from DB filter
      CutsFromDBWireHitFilter m_cutsFromDBFilter;

      /// Switch to MVA super layer from database.
      DBObjPtr<WireHitFilterSettings> m_WireHitFilterSettings;

    };
  }
}
