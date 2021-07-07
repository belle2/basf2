/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
