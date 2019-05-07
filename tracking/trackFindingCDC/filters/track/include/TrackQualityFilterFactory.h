/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/track/BaseTrackFilter.h>
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class FilterFactory<BaseTrackFilter>;

    /** Derived class of TrackFilterFactory with a truth target that also discards clones
     * Probably this whole class could be removed if TrackFilterFactory would be templated with the
     * type of the TruthVarSet as a template argument
     */
    class TrackQualityFilterFactory : public TrackFilterFactory {
    private:
      /// Type of the base class
      using Super = TrackFilterFactory;

    public:
      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseTrackFilter> create(const std::string& filterName) const override;
    };
  }
}
