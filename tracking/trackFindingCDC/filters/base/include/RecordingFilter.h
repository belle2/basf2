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

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AVarSet>
    class RecordingFilter: public FilterOnVarSet<AVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<AVarSet> Super;

    public:
      /// Type of the object to be analysed.
      typedef typename AVarSet::Object Object;

    public:
      /// Constructor of the filter.
      RecordingFilter(const std::string& defaultRootFileName = "records.root",
                      const std::string& defaultTreeName = "records") :
        Super(),
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
        AVarSet& varSet = Super::getVarSet();
        m_recorder.reset(new Recorder(varSet.getAllVariables(), m_param_rootFileName, m_param_treeName));
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        m_recorder->write();
        m_recorder.reset();
        Super::terminate();
      }

    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual Weight operator()(const Object& obj) override final
      {
        Weight extracted = Super::operator()(obj);
        if (not std::isnan(extracted)) {
          m_recorder->capture();
        }

        return m_param_returnWeight;
      }

    private:
      /// Recorder to write all variable sets of the encountered objects.
      std::unique_ptr<Recorder> m_recorder;

      /// Name of the ROOT file to which shall be written.
      std::string m_param_rootFileName;

      /// Name of Tree to be written.
      std::string m_param_treeName;

      /// Returns Weight when this filter is called
      Weight m_param_returnWeight;
    };
  }
}
