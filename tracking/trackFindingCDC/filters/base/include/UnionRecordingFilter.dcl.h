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

#include <tracking/trackFindingCDC/filters/base/FilterFactory.fwd.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {

    /**
     *  A filter that records variables form given objects.
     *  It may record native varsets and/or weigths from other filters.
     */
    template <class AFilter>
    class UnionRecording : public Recording<AFilter> {

    private:
      /// Type of the super class
      using Super = Recording<AFilter>;

      /// Type of the factory used to produce FilterVarSets to be add to the recording
      using AFilterFactory = FilterFactory<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    private:
      /// Type of the factory used to produce FilterVarSets to be add to the recording
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor of the filter.
      UnionRecording(std::unique_ptr<AFilterFactory> filterFactory,
                     const std::string& defaultRootFileName = "records.root",
                     const std::string& defaultTreeName = "records");

      /// Default destructor
      ~UnionRecording();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the recorder before event processing.
      void initialize() override;

      /// Getter for the list of valid names of concret variable sets.
      virtual std::vector<std::string> getValidVarSetNames() const;

      /// Getter for the names of valid filters
      virtual std::vector<std::string> getValidFilterNames() const;

      /// Create a variable set for the given name.
      virtual std::unique_ptr<AVarSet> createVarSet(const std::string& name) const;

      /// Splits the comma separated variable names list into a vector of names.
      const std::vector<std::string>& getVarSetNames() const;

    private:
      /// List of varsets that should be recorded.
      std::vector<std::string> m_param_varSetNames;

      /// Name of the filter to selected to objects to be record.
      std::string m_param_skim = "";

      /// FilterFactory
      std::unique_ptr<AFilterFactory> m_filterFactory;
    };

    /// Convience template to create a recording filter for a filter factory.
    template <class AFilterFactory>
    class UnionRecordingFilter : public UnionRecording<typename AFilterFactory::CreatedFilter> {

    private:
      /// Type of the super class
      using Super = UnionRecording<typename AFilterFactory::CreatedFilter>;

    public:
      /// Constructor of the filter.
      UnionRecordingFilter(const std::string& defaultRootFileName = "records.root",
                           const std::string& defaultTreeName = "records");

      /// Default destructor
      ~UnionRecordingFilter();
    };
  }
}
