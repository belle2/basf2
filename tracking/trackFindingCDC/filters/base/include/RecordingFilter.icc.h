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

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.icc.h>

#include <tracking/trackFindingCDC/mva/Recorder.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /* Recording<> */
    template <class AFilter>
    Recording<AFilter>::Recording(std::unique_ptr<AVarSet> varSet,
                                  const std::string& defaultRootFileName,
                                  const std::string& defaultTreeName)
      : Super(std::move(varSet))
      , m_recorder(nullptr)
      , m_param_rootFileName(defaultRootFileName)
      , m_param_treeName(defaultTreeName)
      , m_param_returnWeight(NAN)
    {}

    template <class AFilter>
    Recording<AFilter>::~Recording() = default;

    template <class AFilter>
    void Recording<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);
      moduleParamList->addParameter(prefixed(prefix, "rootFileName"),
                                    m_param_rootFileName,
                                    "Name of the ROOT file to be written",
                                    m_param_rootFileName);

      moduleParamList->addParameter(prefixed(prefix, "treeName"),
                                    m_param_treeName,
                                    "Name of the Tree to be written",
                                    m_param_treeName);

      moduleParamList->addParameter(prefixed(prefix, "returnWeight"),
                                    m_param_returnWeight,
                                    "Weight this filter should return when called. Defaults to NAN",
                                    m_param_returnWeight);
    }

    template <class AFilter>
    void Recording<AFilter>::initialize()
    {
      if (m_skimFilter) this->addProcessingSignalListener(m_skimFilter.get());
      Super::initialize();
      m_recorder.reset(new Recorder(Super::getVarSet().getNamedVariables(),
                                    m_param_rootFileName,
                                    m_param_treeName));
    }

    template <class AFilter>
    void Recording<AFilter>::terminate()
    {
      m_recorder->write();
      m_recorder.reset();
      Super::terminate();
    }

    template <class AFilter>
    Weight Recording<AFilter>::operator()(const Object& obj)
    {
      if (m_skimFilter) {
        Weight skimWeight = (*m_skimFilter)(obj);
        if (std::isnan(skimWeight)) return NAN;
      }

      Weight extracted = Super::operator()(obj);
      if (not std::isnan(extracted)) {
        m_recorder->capture();
      }

      return m_param_returnWeight;
    }

    template <class AFilter>
    MayBePtr<AFilter> Recording<AFilter>::getSkimFilter() const
    {
      return m_skimFilter.get();
    }

    template <class AFilter>
    void Recording<AFilter>::setSkimFilter(std::unique_ptr<AFilter> skimFilter)
    {
      m_skimFilter = std::move(skimFilter);
    }

    /* RecordingFilter<> */
    template <class AVarSet>
    RecordingFilter<AVarSet>::RecordingFilter(const std::string& defaultRootFileName,
                                              const std::string& defaultTreeName)
      : Super(std::make_unique<AVarSet>(), defaultRootFileName, defaultTreeName)
    {
    }

    template <class AVarSet>
    RecordingFilter<AVarSet>::~RecordingFilter() = default;
  }
}
