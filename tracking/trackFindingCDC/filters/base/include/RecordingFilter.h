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

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/tmva/Recorder.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Filter adapter to make a filter work on a set of variables and record
     *  the observed instances on invokation
     */
    template<class AFilter>
    class Recording : public OnVarSet<AFilter> {

    private:
      /// Type of the base class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    private:
      /// Type of the var set interface to be used
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor of the filter.
      Recording(std::unique_ptr<AVarSet> varSet,
                const std::string& defaultRootFileName = "records.root",
                const std::string& defaultTreeName = "records")
        : Super(std::move(varSet)),
          m_recorder(nullptr),
          m_param_rootFileName(defaultRootFileName),
          m_param_treeName(defaultTreeName),
          m_param_returnWeight(NAN)
      {}

      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override
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

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        if (m_skimFilter) m_skimFilter->initialize();
        m_recorder.reset(new Recorder(Super::getVarSet().getNamedVariables(),
                                      m_param_rootFileName,
                                      m_param_treeName));
      }

      /// Signal the beginning of a new run to the skimming filter and the varset
      virtual void beginRun() override
      {
        if (m_skimFilter) m_skimFilter->beginRun();
        Super::beginRun();
      }

      /// Signal the beginning of a new event to the skimming filter and the varset
      virtual void beginEvent() override
      {
        if (m_skimFilter) m_skimFilter->beginEvent();
        Super::beginEvent();
      }

      /// Signal the end of a new run to the skimming filter and the varset
      virtual void endRun() override
      {
        Super::endRun();
        if (m_skimFilter) m_skimFilter->endRun();
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        m_recorder->write();
        m_recorder.reset();
        if (m_skimFilter) m_skimFilter->terminate();
        Super::terminate();
      }

    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual Weight operator()(const Object& obj) override final
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

    public:
      /// Getter for the skim filter to select objects to be recorded
      MayBePtr<AFilter> getSkimFilter() const
      { return m_skimFilter.get(); }

      /// Setter for the skim filter to select objects to be recorded
      void setSkimFilter(std::unique_ptr<AFilter> skimFilter)
      { m_skimFilter = std::move(skimFilter); }

    private:
      /// Recorder to write all variable sets of the encountered objects.
      std::unique_ptr<Recorder> m_recorder;

      /// Name of the ROOT file to which shall be written.
      std::string m_param_rootFileName;

      /// Name of Tree to be written.
      std::string m_param_treeName;

      /// Returns Weight when this filter is called
      Weight m_param_returnWeight;

      /// Skimming filter to select a subset of objects to be recorded.
      std::unique_ptr<AFilter> m_skimFilter = nullptr;
    };

    /// Filter based on a tmva method.
    template<class AVarSet>
    class RecordingFilter: public Recording<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      typedef Recording<Filter<typename AVarSet::Object> > Super;

    public:
      /// Type of the object to be analysed.
      typedef typename AVarSet::Object Object;

    public:
      /// Constructor of the filter.
      RecordingFilter(const std::string& defaultRootFileName = "records.root",
                      const std::string& defaultTreeName = "records")
        : Super(std::unique_ptr<AVarSet>(new AVarSet()),
                defaultRootFileName,
                defaultTreeName)
      {}
    };
  }
}
