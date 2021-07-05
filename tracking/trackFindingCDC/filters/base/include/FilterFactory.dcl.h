/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterFactory.fwd.h>

#include <map>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Factory that can create apropriate filter instances from a name
    template<class AFilter>
    class FilterFactory {

    public:
      /// Type of the filter that this factory creates.
      using CreatedFilter = AFilter;

    public:
      /// Fill the default filter name values
      FilterFactory(const std::string& defaultFilterName = "");

      /// Make destructor of interface class virtual
      virtual ~FilterFactory();

    public:
      /// Getter for a short identifier of the factory. Currently unused.
      virtual std::string getIdentifier() const = 0;

      /// Getter for a descriptive purpose of the filter.
      virtual std::string getFilterPurpose() const = 0;

      /// Getter for the valid filter names and a description for each
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const = 0;

      /**
       *  Create a filter with the given name, does not set filter specific parameters.
       *
       *  Always emits an error and returns nullptr
       */
      virtual std::unique_ptr<AFilter> create(const std::string& filterName) const;

      /// Create a string with a description mentioning the names of the chooseable filter
      virtual std::string createFiltersNameDescription() const;

      /// Create a string with a description mentioning the parameters of the chooseable filter
      virtual std::string createFiltersParametersDescription() const;

    public:
      /// Legacy - Return the default filter suggested by the factory.
      const std::string& getDefaultFilterName() const;

    private:
      /// Legacy - Default filter name suggested by this factory
      std::string m_defaultFilterName;
    };
  }
}
