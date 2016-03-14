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
#include <framework/core/ModuleParamList.h>

#include <boost/variant.hpp>

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

      /// Type of allowed filter parameters
      using FilterParamVariant =
        boost::variant<bool, int, double, std::string, std::vector<std::string> >;

      /// Type mapping names to filter parameter values
      using FilterParamMap = std::map<std::string, FilterParamVariant>;

      class SetParameterVisitor : public boost::static_visitor<> {
      public:
        SetParameterVisitor(AFilter* filter,
                            ModuleParamList* moduleParamList,
                            std::string paramName)
          : m_filter(filter),
            m_moduleParamList(moduleParamList),
            m_paramName(paramName)
        {}

        void operator()(const std::string& t) const
        {
          B2INFO("Received " << PyObjConvUtils::Type<std::string>::name() << " " << t);
          try {
            m_moduleParamList->getParameter<std::string>(m_paramName).setDefaultValue(t);
          } catch (...) {
            m_filter->setParameter(m_paramName, t);
          }
        }

        template<class T>
        void operator()(const T& t) const
        {
          B2INFO("Received " << PyObjConvUtils::Type<T>::name());
          m_moduleParamList->getParameter<T>(m_paramName).setDefaultValue(t);
        }

      private:
        AFilter* m_filter;
        ModuleParamList* m_moduleParamList;
        std::string m_paramName;
      };

    public:
      /** Fill the default filter name and parameter values*/
      FilterFactoryBase(std::string filterName,
                        FilterParamMap filterParameters
                        = FilterParamMap()) :
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

      /** Create a string with a description for each filter  */
      std::string createFiltersNameDescription() const;

      /** Create a string with a description for the filter parameters */
      std::string createFiltersParametersDescription() const;

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

      /** Return the string holding the used filter name */
      const std::string& getFilterName() const
      { return m_filterName; }

      /** Set the filter name which should be created */
      void setFilterName(const std::string& filterName)
      { m_filterName = filterName; }

      /** Return the map holding the used filter parameters */
      const FilterParamMap& getFilterParameters() const
      { return m_filterParameters; }

    private:
      /**
       * Filter name identifying the name to be constructed.
       */
      std::string m_filterName;

      /**
       * Filter parameters forwarded to the filter
       * Meaning of the Key - Value pairs depend on the concrete filter
       */
      FilterParamMap m_filterParameters;
    };

    template<class AFilter>
    void FilterFactoryBase<AFilter>::exposeParameters(ModuleParamList* moduleParamList,
                                                      const std::string& prefix)
    {
      // Set the module parameters
      moduleParamList->addParameter(prefixed(prefix, "filter"),
                                    m_filterName,
                                    createFiltersNameDescription(),
                                    std::string(m_filterName));

      moduleParamList->addParameter(prefixed(prefix, "filterParameters"),
                                    m_filterParameters,
                                    createFiltersParametersDescription(),
                                    m_filterParameters);
    }

    template<class AFilter>
    std::string FilterFactoryBase<AFilter>::createFiltersNameDescription() const
    {
      // Compose description for the filter names
      std::ostringstream filtersNameDescription;
      filtersNameDescription << getFilterPurpose();
      filtersNameDescription << "Allowed values are: ";
      std::vector<std::string> filterNameAndDescriptions;
      for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
        const std::string& filterName = filterNameAndDescription.first;
        const std::string& filterDescription = filterNameAndDescription.second;
        filterNameAndDescriptions.push_back(quoted(filterName) + " " + bracketed(filterDescription));
      }
      filtersNameDescription << join(", ", filterNameAndDescriptions);
      return filtersNameDescription.str();
    }

    template<class AFilter>
    std::string FilterFactoryBase<AFilter>::createFiltersParametersDescription() const
    {
      // Compose description for the filter parameters
      std::ostringstream filtersParametersDescription;
      filtersParametersDescription << "Key -- value pairs depending on the filter." << std::endl;
      for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
        const std::string& filterName = filterNameAndDescription.first;
        // const std::string& filterDescription = filterNameAndDescription.second;

        std::unique_ptr<AFilter> filter = create(filterName);
        if (not filter) {
          B2WARNING("Could not create a filter for name " << filterName);
          continue;
        }

        /// Old style
        std::map<std::string, std::string> filterParameters = filter->getParameterDescription();

        /// New Style
        ModuleParamList moduleParamList;
        filter->exposeParameters(&moduleParamList);
        for (auto && name : moduleParamList.getParameterNames()) {
          filterParameters[name] = moduleParamList.getParameterDescription(name);
        }

        filtersParametersDescription << quoted(filterName) << " :\n";

        if (filterParameters.empty()) {
          filtersParametersDescription << "(no parameters)";
        } else {
          std::vector<std::string> parameterDescriptions;
          for (const auto& parameterNameAndDescription : filterParameters) {
            const std::string& parameterName = parameterNameAndDescription.first;
            const std::string& parameterDescription = parameterNameAndDescription.second;
            parameterDescriptions.push_back(parameterName + " -- " + parameterDescription);
          }
          filtersParametersDescription << join(",\n", parameterDescriptions);
        }
        filtersParametersDescription << ";\n";
      }
      return filtersParametersDescription.str();
    }

    template<class AFilter>
    std::unique_ptr<AFilter> FilterFactoryBase<AFilter>::create()
    {
      std::unique_ptr<AFilter> ptrFilter = create(m_filterName);

      if (not ptrFilter) {
        // Filter not valid
        std::ostringstream message;
        message << "Unrecognised filter name " << quoted(m_filterName) << ". ";
        message << "Allowed values are: ";
        std::vector<std::string> quotedFilterNames;
        for (const auto& filterNameAndDescription : getValidFilterNamesAndDescriptions()) {
          const std::string& filterName = filterNameAndDescription.first;
          quotedFilterNames.push_back(quoted(filterName));
        }
        message << join(", ", quotedFilterNames);
        message << ".";
        B2ERROR(message.str());
      }

      /// Set parameters
      ModuleParamList moduleParamList;
      ptrFilter->exposeParameters(&moduleParamList);
      for (auto& nameAndValue : m_filterParameters) {
        const std::string paramName = nameAndValue.first;
        const FilterParamVariant& value = nameAndValue.second;
        boost::apply_visitor(SetParameterVisitor(ptrFilter.get(), &moduleParamList, paramName), value);
      }
      return std::move(ptrFilter);

    }

  }
}
