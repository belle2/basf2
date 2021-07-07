/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>

#include <tracking/trackFindingCDC/utilities/Named.h>

#include <framework/logging/Logger.h>

#include <RtypesCore.h>

#include <vector>
#include <string>
#include <memory>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    OnVarSet<AFilter>::OnVarSet(std::unique_ptr<AVarSet> varSet)
      : m_varSet(std::move(varSet))
    {
      B2ASSERT("Varset initialised as nullptr", m_varSet);
    }

    template <class AFilter>
    OnVarSet<AFilter>::~OnVarSet() = default;

    template <class AFilter>
    void OnVarSet<AFilter>::initialize()
    {
      this->addProcessingSignalListener(m_varSet.get());
      Super::initialize();
    }

    template <class AFilter>
    bool OnVarSet<AFilter>::needsTruthInformation()
    {
      bool result = Super::needsTruthInformation();
      if (result) return true;

      const std::vector<Named<Float_t*>>& namedVariables = m_varSet->getNamedVariables();
      for (const Named<Float_t*>& namedVariable : namedVariables) {
        std::string name = namedVariable.getName();
        // If the name contains the word truth it is considered to have Monte carlo information.
        if (name.find("truth") != std::string::npos) {
          return true;
        }
      }
      return false;
    }

    template <class AFilter>
    Weight OnVarSet<AFilter>::operator()(const Object& obj)
    {
      Weight weight = Super::operator()(obj);
      if (std::isnan(weight)) return NAN;
      bool extracted = m_varSet->extract(&obj);
      return extracted ? weight : NAN;
    }

    template <class AFilter>
    auto OnVarSet<AFilter>::releaseVarSet()&& -> std::unique_ptr<AVarSet> {
      return std::move(m_varSet);
    }

    template <class AFilter>
    auto OnVarSet<AFilter>::getVarSet() const -> AVarSet&
    {
      return *m_varSet;
    }

    template <class AFilter>
    void OnVarSet<AFilter>::setVarSet(std::unique_ptr<AVarSet> varSet)
    {
      m_varSet = std::move(varSet);
    }

    template <class AVarSet>
    FilterOnVarSet<AVarSet>::FilterOnVarSet()
      : Super(std::make_unique<AVarSet>())
    {
    }

    template <class AVarSet>
    FilterOnVarSet<AVarSet>::~FilterOnVarSet() = default;
  }
}
