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

#include <tracking/trackFindingCDC/utilities/AddPrefix.h>
#include <framework/core/ModuleParamList.h>

#include <string>
#include <memory>
#include <assert.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
       Factory that can creates apropriate filter instance from parameters.

       It knows about all available filters and their parameters.
       Can collaborate with a Module and expose these parameters to the user in steering file.
    */
    template<class AFilter>
    class FilterFactoryBase {

    public:
      /// Type of the filter that this factory creates.
      typedef AFilter CreatedFilter;

    public:
      /** Fill the default filter name and parameter values*/
      FilterFactoryBase(std::string filterName,
                        std::map<std::string, std::string> filterParameters
                        = std::map<std::string, std::string>()) :
        m_filterName(filterName),
        m_filterParameters(filterParameters)
      {
      }

      virtual ~FilterFactoryBase() { }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList)
      { exposeParameters(moduleParamList, getModuleParamPrefix()); }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

      /** Create a string with a description for the filter names parameter */
      std::string createFilterNamesDescription() const
      {
        // Compose description for the filter names
        std::ostringstream filterNameDescription;
        filterNameDescription << getFilterPurpose();
        filterNameDescription << "Allowed values are: ";
        bool first = true;
        for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
          const std::string& filterName = filterNameAndDescription.first;
          const std::string& filterDescription = filterNameAndDescription.second;

          if (not first) {
            filterNameDescription << ", ";
          }

          filterNameDescription << "\"" << filterName << "\"";
          filterNameDescription << " ";
          filterNameDescription << "(" << filterDescription << ")";
          first = false;

        }

        return filterNameDescription.str();
      }
      /** Create a string with a description for the filter parameters parameter */
      std::string createFilterParametersDescription() const
      {
        // Compose description for the filter parameters
        std::ostringstream filterParametersDescription;
        filterParametersDescription << "Key -- value pairs depending on the filter.\n";
        for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
          const std::string& filterName = filterNameAndDescription.first;
          // const std::string& filterDescription = filterNameAndDescription.second;

          filterParametersDescription << "\"" << filterName << "\"" << " :\n";

          std::unique_ptr<AFilter> filter = create(filterName);
          if (not filter) {
            B2WARNING("Could not create a filter for name " << filterName);
            continue;
          }
          std::map<std::string, std::string> filterParameters = filter->getParameterDescription();
          if (filterParameters.empty()) {
            filterParametersDescription << "(no parameters)";
          } else {
            bool firstParameter = true;
            for (const auto& parameterNameAndDescription : filterParameters) {
              const std::string& parameterName = parameterNameAndDescription.first;
              const std::string& parameterDescription = parameterNameAndDescription.second;
              if (not firstParameter) filterParametersDescription << ",\n";
              filterParametersDescription << " "; // Indent
              filterParametersDescription << parameterName;
              filterParametersDescription << " -- ";
              filterParametersDescription << parameterDescription;
              firstParameter = false;
            }
          }
          filterParametersDescription << ";\n";
        }

        return filterParametersDescription.str();
      }

      /** Return the string holding the used filter name */
      const std::string& getFilterName() const
      {
        return m_filterName;
      }

      /** Set the filter name which should be created */
      void setFilterName(const std::string& filterName)
      {
        m_filterName = filterName;
      }

      /** Return the map holding the used filter parameters */
      const std::map<std::string, std::string>& getFilterParameters() const
      {
        return m_filterParameters;
      }

      /** Create the filter with the currently stored parameters. */
      std::unique_ptr<AFilter> create();

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<AFilter> create(const std::string& /*name*/) const
      {return std::unique_ptr<AFilter>(nullptr); }

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const
      { return ""; }

      /** Getter for the valid filter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const
      { return std::map<std::string, std::string>(); }

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const
      { return ""; }

      /**
      Filter name identifying the name to be constructed.
      */
      std::string m_filterName;

      /**
      Filter parameters forwarded to the filter
      Meaning of the Key - Value pairs depend on the concrete filter
      */
      std::map<std::string, std::string> m_filterParameters;
    };



    template<class AFilter>
    void FilterFactoryBase<AFilter>::exposeParameters(ModuleParamList* moduleParamList,
                                                      const std::string& prefix)
    {
      // Set the module parameters
      moduleParamList->addParameter(addPrefix(prefix, "filter"),
                                    m_filterName,
                                    createFilterNamesDescription(),
                                    std::string(m_filterName));

      moduleParamList->addParameter(addPrefix(prefix, "filterParameters"),
                                    m_filterParameters,
                                    createFilterParametersDescription(),
                                    std::map<std::string, std::string>(m_filterParameters));
    }


    template<class AFilter>
    std::unique_ptr<AFilter> FilterFactoryBase<AFilter>::create()
    {
      std::unique_ptr<AFilter> ptrFilter = create(m_filterName);

      if (not ptrFilter) {
        // Filter not valid
        std::ostringstream message;
        message << "Unrecognised filter name \"" << m_filterName  << "\". ";
        message << "Allowed values are: ";
        bool first = true;
        for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
          const std::string& filterName = filterNameAndDescription.first;
          // const std::string& filterDescription = filterNameAndDescription.second;

          if (not first) {
            message << ", ";
          }
          message << "\"" << filterName << "\"";
          first = false;
        }
        message << ".";
        B2ERROR(message.str());
      } else {
        ptrFilter->setParameters(m_filterParameters);
      }
      return std::move(ptrFilter);
    }

  }
}
