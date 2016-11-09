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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/core/ModuleParamList.h>

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
        m_filter.exposeParameters(moduleParameters, prefix);
      }

      /// Initialize the filter.
      virtual void initialize()
      {
        m_filter.initialize();
        /// Redundant check - should in principle be required by the filter directly
        if (m_filter.needsTruthInformation()) {
          CDCMCManager::getInstance().requireTruthInformation();
        }
      }

      /// Terminate the filter.
      virtual void terminate()
      {
        m_filter.terminate();
      }

    protected:
      /// The filter to be used in the matching
      ChooseableFilter<AFilterFactory> m_filter;
    };
  }
}
