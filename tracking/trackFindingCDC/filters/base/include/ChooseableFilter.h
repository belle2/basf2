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

#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>
#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/core/ModuleParamList.h>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Filter can delegate to a filter chosen and set up at run time by parameters
    template <class AFilter>
    class Chooseable : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    public:
      /// Type of allowed filter parameters
      using FilterParamVariant = boost::variant<bool, int, double, std::string, std::vector<std::string> >;

    public:
      /// Setup the chooseable filter with available choices from the factory
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory)
        : m_param_filterName(filterFactory ? filterFactory->getDefaultFilterName() : "")
        , m_filterFactory(std::move(filterFactory))
      {
        B2ASSERT("Constructing a chooseable filter with no factory", m_filterFactory);
      }

      /// Setup the chooseable filter with available choices from the factory and a default name
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory,
                 const std::string& filterName)
        : m_param_filterName(filterName)
        , m_filterFactory(std::move(filterFactory))
      {
        B2ASSERT("Constructing a chooseable filter with no factory", m_filterFactory);
      }

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        Super::exposeParameters(moduleParamList, prefix);
        if (m_param_filterName == "")
        {
          /// Make a force parameter in case no default was given
          moduleParamList->addParameter(prefixed(prefix, "filter"),
          m_param_filterName,
          m_filterFactory->createFiltersNameDescription());
        } else {
          /// Make a normal parameter in case default is known
          moduleParamList->addParameter(prefixed(prefix, "filter"),
          m_param_filterName,
          m_filterFactory->createFiltersNameDescription(),
          m_param_filterName);
        }
        moduleParamList->addParameter(prefixed(prefix, "filterParameters"),
        m_param_filterParameters,
        m_filterFactory->createFiltersParametersDescription(),
        m_param_filterParameters);
      }

      /// Initialize before event processing.
      void initialize() override
      {
        m_filter = m_filterFactory->create(m_param_filterName);
        if (not m_filter) {
          B2ERROR("Could not create filter with name " << m_param_filterName);
          return;
        }

        /// Transfer parameters
        ModuleParamList moduleParamList;
        const std::string prefix = "";
        m_filter->exposeParameters(&moduleParamList, prefix);
        AssignParameterVisitor::update(&moduleParamList, m_param_filterParameters);
        this->addProcessingSignalListener(m_filter.get());
        Super::initialize();
      }

      /// Indicates if the filter requires Monte Carlo information.
      bool needsTruthInformation() override
      {
        return m_filter->needsTruthInformation();
      }

      /**
       *  Function to evaluate the object.
       *  Delegates to the filter chosen by module parameters.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      Weight operator()(const Object& object) final {
        return (*m_filter)(object);
      }

    private:
      /// Parameters : Name of the selected filter
      std::string m_param_filterName;

      /// Parameter: Parameter keys and values to be forwarded to the chosen filter
      std::map<std::string, FilterParamVariant> m_param_filterParameters;

      /// Filter factor to construct a chosen filter
      std::unique_ptr<FilterFactory<AFilter>> m_filterFactory = nullptr;

      /// Chosen filter
      std::unique_ptr<AFilter> m_filter = nullptr;
    };

    /// Convenvience wrapper to setup a Chooseable filter from a specific factory object.
    template <class AFilterFactory>
    class ChooseableFilter : public Chooseable<typename AFilterFactory::CreatedFilter> {

    private:
      /// Type of the super class
      using Super = Chooseable<typename AFilterFactory::CreatedFilter>;

    public:
      /// Constructor of the chooseable filter taking the default filter name and parameters
      ChooseableFilter()
        : Super(makeUnique<AFilterFactory>())
      {
      }

      /// Constructor of the chooseable filter taking the default filter name and parameters
      ChooseableFilter(const std::string& filterName)
        : Super(makeUnique<AFilterFactory>(), filterName)
      {
      }
    };
  }
}
