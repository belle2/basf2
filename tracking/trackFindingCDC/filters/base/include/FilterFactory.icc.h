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

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    FilterFactory<AFilter>::FilterFactory(const std::string& defaultFilterName)
      : m_defaultFilterName(defaultFilterName)
    {
    }

    template <class AFilter>
    FilterFactory<AFilter>::~FilterFactory() = default;

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

    template<class AFilter>
    const std::string& FilterFactory<AFilter>::getDefaultFilterName() const
    {
      return m_defaultFilterName;
    }
  }
}
