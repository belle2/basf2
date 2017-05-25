/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

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
      FilterFactory(const std::string& defaultFilterName = "")
        : m_defaultFilterName(defaultFilterName)
      {
      }

      /// Make destructor of interface class virtual
      virtual ~FilterFactory() = default;

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
       *  Always emits an error message and returns nullptr.
       */
      virtual std::unique_ptr<AFilter> create(const std::string& filterName) const;

      /// Create a string with a description mentioning the names of the chooseable filter
      virtual std::string createFiltersNameDescription() const;

      /// Create a string with a description mentioning the parameters of the chooseable filter
      virtual std::string createFiltersParametersDescription() const;

    public:
      /// Legacy - Return the default filter suggested by the factory.
      const std::string& getDefaultFilterName() const
      {
        return m_defaultFilterName;
      }

    private:
      /// Legacy - Default filter name suggested by this factory
      std::string m_defaultFilterName;
    };

    template <class AFilter>
    std::unique_ptr<AFilter> FilterFactory<AFilter>::create(const std::string& filterName) const
    {
      // Filter not valid
      B2ERROR("Could not create filter with name " << filterName);
      std::ostringstream message;
      message << "Known filter names are: ";
      std::vector<std::string> quotedFilterNames;
      for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
        const std::string& validFilterName = filterNameAndDescription.first;
        quotedFilterNames.push_back(quoted(validFilterName));
      }
      message << join(", ", quotedFilterNames);
      message << ".";
      B2ERROR(message.str());
      return std::unique_ptr<AFilter>(nullptr);
    }

    template<class AFilter>
    std::string FilterFactory<AFilter>::createFiltersNameDescription() const
    {
      // Compose description for the filter names
      std::ostringstream oss;
      oss << this->getFilterPurpose();
      oss << "Allowed values are: ";
      std::vector<std::string> filterNameAndDescriptions;
      for (const auto& filterNameAndDescription : this->getValidFilterNamesAndDescriptions()) {
        const std::string& filterName = filterNameAndDescription.first;
        const std::string& filterDescription = filterNameAndDescription.second;
        filterNameAndDescriptions.push_back(quoted(filterName) + " " + bracketed(filterDescription));
      }
      oss << join(", ", filterNameAndDescriptions);
      return oss.str();
    }

    template<class AFilter>
    std::string FilterFactory<AFilter>::createFiltersParametersDescription() const
    {
      // Compose description for the filter parameters
      std::ostringstream oss;
      oss << "Key -- value pairs depending on the filter." << std::endl;
      for (const auto& filterNameAndDescription : this->getValidFilterNamesAndDescriptions()) {

        const std::string& filterName = filterNameAndDescription.first;
        // const std::string& filterDescription = filterNameAndDescription.second;

        std::unique_ptr<AFilter> filter = this->create(filterName);
        if (not filter) {
          B2WARNING("Could not create a filter for name " << filterName);
          continue;
        }

        ModuleParamList moduleParamList;
        const std::string prefix = "";
        filter->exposeParameters(&moduleParamList, prefix);

        std::map<std::string, std::string> filterParameters;
        for (auto && name : moduleParamList.getParameterNames()) {
          filterParameters[name] = moduleParamList.getParameterDescription(name);
        }

        oss << quoted(filterName) << " :\n";
        if (filterParameters.empty()) {
          oss << "(no parameters)";
        } else {
          std::vector<std::string> parameterDescriptions;
          for (const auto& parameterNameAndDescription : filterParameters) {
            const std::string& parameterName = parameterNameAndDescription.first;
            const std::string& parameterDescription = parameterNameAndDescription.second;
            parameterDescriptions.push_back(parameterName + " -- " + parameterDescription);
          }
          oss << join(",\n", parameterDescriptions);
        }
        oss << ";\n";
      }
      return oss.str();
    }
  }
}
