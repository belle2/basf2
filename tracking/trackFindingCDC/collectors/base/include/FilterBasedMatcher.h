/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// A Matcher Algorithm for adding collection items to collector items using a configurable filter (e.g. TMVA).
    template<class AFilterFactory>
    class FilterBasedMatcher {
    public:
      /// Empty desctructor. Everything is handled via terminate.
      virtual ~FilterBasedMatcher() = default;

      /// Expose the parameters to the module.
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "")
      {
        m_filterFactory.exposeParameters(moduleParameters, prefix);
      }

      /// Initialize the filter.
      virtual void initialize()
      {
        m_filter = m_filterFactory.create();
        m_filter->initialize();

        if (m_filter->needsTruthInformation()) {
          StoreArray <CDCSimHit>::required();
          StoreArray <MCParticle>::required();
        }
      }

      /// Terminate the filter.
      virtual void terminate()
      {
        m_filter->terminate();
      }

    protected:
      /// The filter factory will output this filter later.
      std::unique_ptr<typename AFilterFactory::CreatedFilter> m_filter;

    private:
      /// Use this filter factory.
      AFilterFactory m_filterFactory;
    };
  }
}
