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

#include <tracking/trackFindingCDC/filters/base/FilterFactory.fwd.h>

#include <tracking/trackFindingCDC/filters/base/FilterParamMap.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace Belle2 {
  class ModuleParamList;

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
      /// Setup the chooseable filter with available choices from the factory
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory);

      /// Setup the chooseable filter with available choices from the factory and a default name
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory,
                 const std::string& filterName);

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize before event processing.
      void initialize() override;

      /// Indicates if the filter requires Monte Carlo information.
      bool needsTruthInformation() override;

      /**
       *  Function to evaluate the object.
       *  Delegates to the filter chosen by module parameters.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      Weight operator()(const Object& object) final;

    private:
      /// Parameters : Name of the selected filter
      std::string m_param_filterName;

      /// Parameter: Parameter keys and values to be forwarded to the chosen filter
      FilterParamMap m_param_filterParameters;
      // std::map<std::string, FilterParamVariant> m_param_filterParameters;

      /// Filter factor to construct a chosen filter
      std::unique_ptr<FilterFactory<AFilter> > m_filterFactory;

      /// Chosen filter
      std::unique_ptr<AFilter> m_filter;
    };

    /**
     *  Convenvience wrapper to setup a Chooseable filter from a specific factory object.
     *
     *  @deprecated Try to use Chooseable<AFilter>(factory) and dependency injection.
     */
    template <class AFilterFactory>
    class ChooseableFilter
      : public Chooseable<typename AFilterFactory::CreatedFilter> {

    private:
      /// Type of the super class
      using Super = Chooseable<typename AFilterFactory::CreatedFilter>;

    public:
      /// Constructor of the chooseable filter
      ChooseableFilter();

      /// Constructor of the chooseable filter taking the default filter name
      ChooseableFilter(const std::string& filterName);
    };
  }
}
