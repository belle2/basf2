/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.icc.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    ChoosableFromVarSet<AFilter>::ChoosableFromVarSet(std::unique_ptr<AVarSet> varSet,
                                                      std::string varName)
      : Super(std::move(varSet))
      , m_param_varName(varName)
    {
    }

    template <class AFilter>
    ChoosableFromVarSet<AFilter>::~ChoosableFromVarSet() = default;

    template <class AFilter>
    void ChoosableFromVarSet<AFilter>::exposeParameters(ModuleParamList* moduleParamList,
                                                        const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);

      if (m_param_varName == "") {
        // Make a forced parameter if no default variable name is present
        moduleParamList->addParameter(prefixed(prefix, "chosenVariable"),
                                      m_param_varName,
                                      "Choose the name of the variable "
                                      "that will be put out as a weight.");
      } else {
        // Normal unforced parameter if default name is present
        moduleParamList->addParameter(prefixed(prefix, "chosenVariable"),
                                      m_param_varName,
                                      "Choose the name of the variable "
                                      "that will be put out as a weight.",
                                      m_param_varName);
      }
    }

    template <class AFilter>
    void ChoosableFromVarSet<AFilter>::initialize()
    {
      Super::initialize();
      MayBePtr<Float_t> foundVariable = Super::getVarSet().find(m_param_varName);
      if (not foundVariable) {
        B2ERROR("Could not find request variable name " << m_param_varName << " in variable set");
        B2INFO("Valid names are: ");
        std::vector<Named<Float_t*>> namedVariables = Super::getVarSet().getNamedVariables();
        for (const Named<Float_t*>& namedVariable : namedVariables) {
          std::string name = namedVariable.getName();
          B2INFO("* " << name);
        }
      }
      m_variable = foundVariable;
    }

    template <class AFilter>
    Weight ChoosableFromVarSet<AFilter>::operator()(const Object& object)
    {
      Weight extracted = Super::operator()(object);
      if (std::isnan(extracted)) {
        return NAN;
      } else {
        if (m_variable) {
          return *m_variable;
        } else {
          return NAN;
        }
      }
    }

    template <class AVarSet>
    ChoosableFromVarSetFilter<AVarSet>::ChoosableFromVarSetFilter(const std::string& varName)
      : Super(std::make_unique<AVarSet>(), varName)
    {
    }

    template <class AVarSet>
    ChoosableFromVarSetFilter<AVarSet>::~ChoosableFromVarSetFilter() = default;
  }
}
