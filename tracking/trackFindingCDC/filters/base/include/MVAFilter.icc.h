/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/mva/MVAExpert.h>

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.icc.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/utilities/Named.h>

#include <RtypesCore.h>

#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    template <class AFilter>
    MVA<AFilter>::MVA(std::unique_ptr<AVarSet> varSet,
                      const std::string& identifier,
                      double defaultCut)
      : Super(std::move(varSet))
      , m_param_cut(defaultCut)
      , m_param_identifier(identifier)
    {
    }

    template <class AFilter>
    MVA<AFilter>::~MVA() = default;

    template <class AFilter>
    void MVA<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);
      moduleParamList->addParameter(prefixed(prefix, "cut"),
                                    m_param_cut,
                                    "The cut value of the mva output below which the object is rejected",
                                    m_param_cut);

      moduleParamList->addParameter(prefixed(prefix, "identifier"),
                                    m_param_identifier,
                                    "Database identfier of the expert of weight file name",
                                    m_param_identifier);
    }

    template <class AFilter>
    void MVA<AFilter>::initialize()
    {
      Super::initialize();
      std::vector<Named<Float_t*>> namedVariables = Super::getVarSet().getNamedVariables();
      m_mvaExpert = std::make_unique<MVAExpert>(m_param_identifier, std::move(namedVariables));
      m_mvaExpert->initialize();
    }

    template <class AFilter>
    void MVA<AFilter>::beginRun()
    {
      Super::beginRun();
      m_mvaExpert->beginRun();
    }

    template <class AFilter>
    Weight MVA<AFilter>::operator()(const Object& obj)
    {
      double prediction = predict(obj);
      return prediction < m_param_cut ? NAN : prediction;
    }

    template <class AFilter>
    double MVA<AFilter>::predict(const Object& obj)
    {
      Weight extracted = Super::operator()(obj);
      if (std::isnan(extracted)) {
        return NAN;
      } else {
        return m_mvaExpert->predict();
      }
    }

    template <class AVarSet>
    MVAFilter<AVarSet>::MVAFilter(const std::string& defaultTrainingName,
                                  double defaultCut)
      : Super(std::make_unique<AVarSet>(), defaultTrainingName, defaultCut)
    {
    }

    template <class AVarSet>
    MVAFilter<AVarSet>::~MVAFilter() = default;
  }
}
