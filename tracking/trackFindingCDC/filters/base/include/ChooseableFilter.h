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
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter can delegate its decision to a filter choosen and set up at run time by parameters from a module
    template<class AFilterFactory>
    class ChooseableFilter: public AFilterFactory::CreatedFilter {

    private:
      /// Type of the super class
      typedef typename AFilterFactory::CreatedFilter Super;

    public:
      /// Type of the object to be analysed.
      typedef typename AFilterFactory::CreatedFilter::Object Object;

    public:
      /// Constructor of the chooseable filter taking the default filter name and parameters
      ChooseableFilter() :
        m_filterFactory()
      {}


      /// Constructor of the chooseable filter taking the default filter name and parameters
      ChooseableFilter(std::string filterName,
                       std::map<std::string, std::string> filterParameters
                       = std::map<std::string, std::string>()) :
        m_filterFactory(filterName, filterParameters)
      {}

      /** Expose the set of parameters of the filter to the module parameter list.
       *
       *  Note that not all filters have yet exposed their parameters in this way.
       */
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        Super::exposeParameters(moduleParamList, prefix);
        m_filterFactory.exposeParameters(moduleParamList, prefix);
      }

      /** Return the string holding the used filter name */
      const std::string& getFilterName() const
      {
        return m_filterFactory.getFilterName();
      }

      /** Set the filter name which should be created */
      void setFilterName(const std::string& filterName)
      {
        m_filterFactory.setFilterName(filterName);
      }

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        m_filter = m_filterFactory.create();
        m_filter->initialize();
        Super::initialize();

        if (needsTruthInformation()) {
          CDCMCManager::getInstance().requireTruthInformation();
        }

      }

      /// Indicates if the filter requires Monte Carlo information.
      virtual bool needsTruthInformation() override
      {
        return m_filter->needsTruthInformation();
      }

      /// Signal the beginning of a new event
      virtual void beginEvent() override
      {
        B2ASSERT("No filter was set up. Forgot to initialise the ChooseableFilter", m_filter);
        m_filter->beginEvent();
        Super::beginEvent();

        if (needsTruthInformation()) {
          CDCMCManager::getInstance().fill();
        }
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        Super::terminate();
        m_filter->terminate();
      }

      /** Function to evaluate the object.
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
      /// FilterFactory
      AFilterFactory m_filterFactory;

      /// Choosen filter
      std::unique_ptr<typename AFilterFactory::CreatedFilter> m_filter = nullptr;

    };
  }
}
