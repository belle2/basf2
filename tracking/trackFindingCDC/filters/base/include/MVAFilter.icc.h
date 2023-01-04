/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
                      double defaultCut,
                      const std::string& dbObjectName)
      : Super(std::move(varSet)), m_identifier(identifier), m_cutValue(defaultCut), m_DBPayloadName(dbObjectName)
    {
    }

    template <class AFilter>
    MVA<AFilter>::~MVA() = default;

    template <class AFilter>
    void MVA<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);
      moduleParamList->addParameter(prefixed(prefix, "cut"),
                                    m_cutValue,
                                    "The cut value of the mva output below which the object is rejected",
                                    m_cutValue);

      moduleParamList->addParameter(prefixed(prefix, "identifier"),
                                    m_identifier,
                                    "Database identfier of the expert of weight file name",
                                    m_identifier);

      moduleParamList->addParameter(prefixed(prefix, "DBPayloadName"),
                                    m_DBPayloadName,
                                    "Name of the DB payload containing weightfile name and the cut value. If a DB payload with both values is available and valid, it will override the values provided by parameters.",
                                    m_DBPayloadName);
    }

    template <class AFilter>
    void MVA<AFilter>::initialize()
    {
      Super::initialize();

      m_mvaPayload = std::make_unique<OptionalDBObjPtr<BaseTrackingMVAFilterPayload>>(m_DBPayloadName);
      if (m_mvaPayload->isValid()) {
        m_identifier = (*m_mvaPayload)->getIdentifierName();
        m_cutValue = (*m_mvaPayload)->getCutValue();
        B2DEBUG(20,
                "MVAFilter: Using DBObject " << m_DBPayloadName << " with weightfile " << m_identifier << " and cut value " << m_cutValue << ".");
      }

      std::vector<Named<Float_t*>> namedVariables = Super::getVarSet().getNamedVariables();
      m_mvaExpert = std::make_unique<MVAExpert>(m_identifier, std::move(namedVariables));
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
      return prediction < m_cutValue ? NAN : prediction;
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
                                  double defaultCut,
                                  const std::string& defaultDBObjectName)
      : Super(std::make_unique<AVarSet>(), defaultTrainingName, defaultCut, defaultDBObjectName)
    {
    }

    template <class AVarSet>
    MVAFilter<AVarSet>::~MVAFilter() = default;
  }
}
