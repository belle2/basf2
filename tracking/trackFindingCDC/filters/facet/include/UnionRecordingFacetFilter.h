/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/FacetFilterFactory.h>

#include <tracking/trackingUtilities/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackingUtilities/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCFacet;
  }
  namespace TrackFindingCDC {

    /// Filter to record multiple chooseable variable sets for facets
    class UnionRecordingFacetFilter: public TrackingUtilities::UnionRecordingFilter<FacetFilterFactory> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::UnionRecordingFilter<FacetFilterFactory>;

    public:
      /// Get the valid names of variable sets for facets.
      std::vector<std::string> getValidVarSetNames() const final;

      /// Create a concrete variables set for facets from a name.
      std::unique_ptr<TrackingUtilities::BaseVarSet<const TrackingUtilities::CDCFacet>> createVarSet(const std::string& name) const final;
    };
  }
}
