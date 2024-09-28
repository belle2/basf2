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
#include <mva/interface/Dataset.h>
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

      DBObjPtr<TrackingMVAFilterParameters> mvaParameterPayload(m_DBPayloadName);
      if (mvaParameterPayload.isValid()) {
        m_identifier = mvaParameterPayload->getIdentifierName();
        m_cutValue = mvaParameterPayload->getCutValue();
        B2DEBUG(20, "MVAFilter: Using DBObject " << m_DBPayloadName << " with weightfile " << m_identifier << " and cut value " <<
                m_cutValue << ".");
      } else {
        B2FATAL("MVAFilter: No valid MVAFilter payload with name " + m_DBPayloadName + " was found.");
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
      /// Make sure that the sequence of columns (features) is correct and follows the one from the weightFile
      const auto& selectedVars = m_mvaExpert->getVariableNames();
      const std::vector<Named<Float_t*>>& namedVariables = Super::getVarSet().getNamedVariables();
      m_namedVariables.clear();
      for (const auto& name : selectedVars) {

        auto itNamedVariable = std::find_if(namedVariables.begin(),
                                            namedVariables.end(),
        [name](const Named<Float_t*>& namedVariable) {
          return namedVariable.getName() == name;
        });
        if (itNamedVariable == namedVariables.end()) {
          B2ERROR("Variable name " << name << " mismatch for MVA filter. " <<
                  "Could not find expected variable '" << name << "'");
        }
        m_namedVariables.push_back(*itNamedVariable);
      }
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

    template <class AFilter>
    std::vector<float> MVA<AFilter>::predict(const std::vector<Object*>& objs)
    {
      const int nFeature = m_namedVariables.size();
      const int nRows    = objs.size();
      auto allFeatures = std::unique_ptr<float[]>(new float[nRows * nFeature]);
      size_t iRow = 0;
      for (const auto& obj : objs) {
        if (Super::getVarSet().extract(obj)) {
          for (int iFeature = 0; iFeature < nFeature; iFeature += 1) {
            allFeatures[nFeature * iRow + iFeature] = *m_namedVariables[iFeature];
          }
          iRow += 1;
        }
      }
      return m_mvaExpert->predict(allFeatures.get(), nFeature, nRows);
    }

    template <class AFilter>
    std::vector<float> MVA<AFilter>::operator()(const std::vector<Object*>& objs)
    {
      auto out = predict(objs);
      for (auto& res : out) {
        res = res < m_cutValue ? NAN : res;
      }
      return out;
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
