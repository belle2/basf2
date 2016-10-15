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

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter can delegate to a filter coosen and set up at run time by parameters
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
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory)
        : m_filterFactory(std::move(filterFactory))
      {
      }

      /// Setup the chooseable filter with available choices from the factory and a default name
      Chooseable(std::unique_ptr<FilterFactory<AFilter>> filterFactory,
                 const std::string& filterName)
        : m_filterFactory(std::move(filterFactory))
      {
        B2ASSERT("Constructing a chooseable filter with no factory", filterFactory);
        setFilterName(filterName);
      }

      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override final
      {
        Super::exposeParameters(moduleParamList, prefix);
        m_filterFactory->exposeParameters(moduleParamList);
      }

      /// Return the string holding the used filter name
      const std::string& getFilterName() const
      {
        return m_filterFactory->getFilterName();
      }

      /// Set the filter name which should be created
      void setFilterName(const std::string& filterName)
      {
        m_filterFactory->setFilterName(filterName);
      }

      /// Initialize before event processing.
      virtual void initialize() override
      {
        if (not m_filterFactory) {
          B2ERROR("Filter factory not setup");
          return;
        }

        m_filter = m_filterFactory->create();
        if (m_filter) {
          m_filter->initialize();
        } else {
          B2ERROR("Could not create filter with name " << getFilterName());
        }
        Super::initialize();
      }

      /// Indicates if the filter requires Monte Carlo information.
      virtual bool needsTruthInformation() override
      {
        return m_filter->needsTruthInformation();
      }

      /// Signal the beginning of a new run
      virtual void beginRun() override
      {
        m_filter->beginRun();
        Super::beginRun();
      }

      /// Signal the beginning of a new event
      virtual void beginEvent() override
      {
        B2ASSERT("No filter was set up. Forgot to initialise the ChooseableFilter", m_filter);
        m_filter->beginEvent();
        Super::beginEvent();
      }

      /// Signal the end of a run
      virtual void endRun() override
      {
        Super::endRun();
        m_filter->endRun();
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        Super::terminate();
        m_filter->terminate();
      }

      /**
       *  Function to evaluate the object.
       *  Delegates to the filter choosen by module parameters.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      virtual Weight operator()(const Object& object) override final
      {
        return (*m_filter)(object);
      }

    private:
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
