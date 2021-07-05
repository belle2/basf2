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

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>
#include <memory>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class Recorder;

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
                const std::string& defaultTreeName = "records");

      /// Default destructor
      ~Recording();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the recorder before event processing.
      void initialize() override;

      /// Initialize the recorder after event processing.
      void terminate() override;

    public:
      /// Function to evaluate the cluster for its backgroundness.
      Weight operator()(const Object& obj) final;

    protected:
      /// Getter for the skim filter to select objects to be recorded
      MayBePtr<AFilter> getSkimFilter() const;

      /// Setter for the skim filter to select objects to be recorded
      void setSkimFilter(std::unique_ptr<AFilter> skimFilter);

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

    /// Convience template to create a recording filter for a set of variables.
    template<class AVarSet>
    class RecordingFilter: public Recording<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      using Super = Recording<Filter<typename AVarSet::Object> >;

    public:
      /// Type of the object to be analysed.
      using Object = typename AVarSet::Object;

    public:
      /// Constructor of the filter.
      explicit RecordingFilter(const std::string& defaultRootFileName = "records.root",
                               const std::string& defaultTreeName = "records");

      /// Default destructor
      ~RecordingFilter();
    };
  }
}
