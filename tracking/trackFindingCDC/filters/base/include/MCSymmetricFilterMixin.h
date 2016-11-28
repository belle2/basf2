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

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <map>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin for filters that use Monte Carlo information.
    template<class AFilter>
    class MCSymmetricFilterMixin : public AFilter {

    private:
      /// Type of the super class
      using Super = AFilter;

    public:
      /// Constructor
      explicit MCSymmetricFilterMixin(bool allowReverse = true) :
        Super(),
        m_param_allowReverse(allowReverse)
      {
      }

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "allowReverse"),
                                      m_param_allowReverse,
                                      "Indication whether the reverse to the truth is accepted"
                                      "preserving the progagation reversal symmetry.",
                                      m_param_allowReverse);

      }

      /// Initialize the before event processing.
      void initialize() override
      {
        CDCMCManager::getInstance().requireTruthInformation();
        Super::initialize();
      }

      /// Signal the beginning of a new event
      void beginEvent() override
      {
        CDCMCManager::getInstance().fill();
        Super::beginEvent();
      }

      /// Indicates that the filter requires Monte Carlo information.
      bool needsTruthInformation() final {
        return true;
      }

    public:
      /// Setter for the allow reverse parameter
      virtual void setAllowReverse(bool allowReverse)
      { m_param_allowReverse = allowReverse; }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      { return m_param_allowReverse; }

    private:
      /// Switch to indicate if the reversed version of the object shall also be accepted (default is true).
      bool m_param_allowReverse;
    };
  }
}
