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

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {

    template <class AFilter>
    MCSymmetric<AFilter>::MCSymmetric(bool allowReverse)
      : Super()
      , m_param_allowReverse(allowReverse)
    {
    }

    template <class AFilter>
    MCSymmetric<AFilter>::~MCSymmetric() = default;

    template <class AFilter>
    void MCSymmetric<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);
      moduleParamList->addParameter(prefixed(prefix, "allowReverse"),
                                    m_param_allowReverse,
                                    "Indication whether the reverse to the truth is accepted"
                                    "preserving the progagation reversal symmetry.",
                                    m_param_allowReverse);
    }

    template <class AFilter>
    void MCSymmetric<AFilter>::initialize()
    {
      CDCMCManager::getInstance().requireTruthInformation();
      Super::initialize();
    }

    template <class AFilter>
    void MCSymmetric<AFilter>::beginEvent()
    {
      CDCMCManager::getInstance().fill();
      Super::beginEvent();
    }

    template <class AFilter>
    bool MCSymmetric<AFilter>::needsTruthInformation()
    {
      return true;
    }

    template <class AFilter>
    void MCSymmetric<AFilter>::setAllowReverse(bool allowReverse)
    {
      m_param_allowReverse = allowReverse;
    }

    template <class AFilter>
    bool MCSymmetric<AFilter>::getAllowReverse() const
    {
      return m_param_allowReverse;
    }
  }
}
