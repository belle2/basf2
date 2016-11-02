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

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/FilterVarSet.h>

#include <tracking/trackFindingCDC/varsets/UnionVarSet.h>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A filter that records variables form given objects. It may record native varsets and returned weigths from other filters.
    template<class AFilterFactory>
    class UnionRecordingFilter:
      public Recording<typename AFilterFactory::CreatedFilter> {

    private:
      /// Type of the super class
      using Super = Recording<typename AFilterFactory::CreatedFilter>;

      /// Type of the filters that can be included in the recodring
      using CreatedFilter = typename AFilterFactory::CreatedFilter;

    public:
      /// Type of the object to be analysed.
      using Object = typename CreatedFilter::Object;

    public:
      /// Constructor of the filter.
      UnionRecordingFilter(const std::string& defaultRootFileName = "records.root",
                           const std::string& defaultTreeName = "records") :
        Super(std::unique_ptr<UnionVarSet<Object> >(new UnionVarSet<Object>),
              defaultRootFileName,
              defaultTreeName),
        m_filterFactory("truth")
      {}

      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override
      {
        Super::exposeParameters(moduleParamList, prefix);

        moduleParamList->addParameter(prefixed(prefix, "varSets"),
                                      m_param_varSetNames,
                                      "List of names refering to concrete variable sets."
                                      "Valid names: " + join(", ", getValidVarSetNames()),
                                      m_param_varSetNames);

        moduleParamList->addParameter(prefixed(prefix, "skim"),
                                      m_param_skim,
                                      "Filter name which object must pass to be recorded."
                                      "Valid names: " + join(", ", getValidFilterNames()),
                                      m_param_skim);
      }

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        /// Create the skimming filter
        if (m_param_skim != "") {
          std::unique_ptr<CreatedFilter> skimFilter = m_filterFactory.create(m_param_skim);
          this->setSkimFilter(std::move(skimFilter));
        }

        std::unique_ptr<UnionVarSet<Object> > multiVarSet(new UnionVarSet<Object>());

        /// Create and add the concrete varsets from the varset parameter.
        for (std::string name : getVarSetNames()) {
          std::unique_ptr<BaseVarSet<Object> > varSet = createVarSet(name);
          if (varSet) {
            multiVarSet->push_back(std::move(varSet));
          } else {
            B2WARNING("Could not create a variable set from name " << name);
          }
        }
        Super::setVarSet(std::move(multiVarSet));
        Super::initialize();
      }

      /// Getter for the list of valid names of concret variable sets.
      virtual std::vector<std::string> getValidVarSetNames() const
      {
        // Get all filter names and make a var set name for each.
        std::vector<std::string> varSetNames;
        std::vector<std::string> filterNames = getValidFilterNames();
        for (const std::string& filterName : filterNames) {
          std::string varSetName = "filter(" + filterName + ")";
          varSetNames.push_back(varSetName);
        }
        return varSetNames;
      }

      /// Getter for the names of valid filters
      virtual std::vector<std::string> getValidFilterNames() const
      {
        // Get all filter names and make a var set name for each.
        std::map<std::string, std::string> filterNamesAndDescriptions =
          m_filterFactory.getValidFilterNamesAndDescriptions();

        std::vector<std::string> filterNames;
        filterNames.reserve(filterNamesAndDescriptions.size());
        for (const std::pair<std::string, std::string>& filterNameAndDescription :
             filterNamesAndDescriptions) {
          const std::string& filterName = filterNameAndDescription.first;
          filterNames.push_back(filterName);
        }
        return filterNames;
      }

      /// Create a variable set for the given name.
      virtual std::unique_ptr<BaseVarSet<Object>> createVarSet(const std::string& name) const
      {
        if (boost::starts_with(name, "filter(") and boost::ends_with(name, ")")) {
          B2INFO("Detected filter name");
          std::string filterName = name.substr(7, name.size() - 8);
          B2INFO("filterName = " << filterName);
          std::unique_ptr<CreatedFilter> filter = m_filterFactory.create(filterName);
          if (not filter) {
            B2WARNING("Could not construct filter for name " << filterName);
            return std::unique_ptr<BaseVarSet<Object>>(nullptr);
          } else {
            BaseVarSet<Object>* filterVarSet =
              new FilterVarSet<CreatedFilter>(filterName, std::move(filter));
            return std::unique_ptr<BaseVarSet<Object> >(filterVarSet);
          }
        } else {
          return std::unique_ptr<BaseVarSet<Object> >(nullptr);
        }
      }

      /// Splits the comma separated variable names list into a vector of names.
      const std::vector<std::string>& getVarSetNames() const
      {
        return m_param_varSetNames;
      }

    private:
      /// List of varsets that should be recorded.
      std::vector<std::string> m_param_varSetNames;

      /// Name of the filter to selected to objects to be record.
      std::string m_param_skim = "";

      /// FilterFactory
      AFilterFactory m_filterFactory;

    };
  }
}
