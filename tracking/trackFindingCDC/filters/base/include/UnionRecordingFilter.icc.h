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

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterVarSet.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterVarSet.dcl.h>

#include <tracking/trackFindingCDC/varsets/UnionVarSet.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /* UnionRecording<> */
    template <class AFilter>
    UnionRecording<AFilter>::UnionRecording(std::unique_ptr<AFilterFactory> filterFactory,
                                            const std::string& defaultRootFileName,
                                            const std::string& defaultTreeName)
      : Super(std::make_unique<UnionVarSet<Object>>(), defaultRootFileName, defaultTreeName)
      , m_filterFactory(std::move(filterFactory))
    {
    }

    template <class AFilter>
    UnionRecording<AFilter>::~UnionRecording() = default;

    template <class AFilter>
    void UnionRecording<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(prefixed(prefix, "varSets"),
                                    m_param_varSetNames,
                                    "List of names refering to concrete variable sets."
                                    "Valid names: " +
                                    join(", ", this->getValidVarSetNames()),
                                    m_param_varSetNames);

      moduleParamList->addParameter(prefixed(prefix, "skim"),
                                    m_param_skim,
                                    "Filter name which object must pass to be recorded."
                                    "Valid names: " +
                                    join(", ", this->getValidFilterNames()),
                                    m_param_skim);
    }

    template <class AFilter>
    void UnionRecording<AFilter>::initialize()
    {
      /// Create the skimming filter
      if (m_param_skim != "") {
        std::unique_ptr<AFilter> skimFilter = m_filterFactory->create(m_param_skim);
        this->setSkimFilter(std::move(skimFilter));
      }

      auto multiVarSet = std::make_unique<UnionVarSet<Object>>();

      /// Create and add the concrete varsets from the varset parameter.
      for (std::string name : getVarSetNames()) {
        std::unique_ptr<AVarSet> varSet = createVarSet(name);
        if (varSet) {
          multiVarSet->push_back(std::move(varSet));
        } else {
          B2WARNING("Could not create a variable set from name " << name);
        }
      }
      this->setVarSet(std::move(multiVarSet));
      Super::initialize();
    }

    template <class AFilter>
    std::vector<std::string> UnionRecording<AFilter>::getValidVarSetNames() const
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

    template <class AFilter>
    std::vector<std::string> UnionRecording<AFilter>::getValidFilterNames() const
    {
      // Get all filter names and make a var set name for each.
      std::map<std::string, std::string> filterNamesAndDescriptions =
        m_filterFactory->getValidFilterNamesAndDescriptions();

      std::vector<std::string> filterNames;
      filterNames.reserve(filterNamesAndDescriptions.size());
      for (const std::pair<std::string, std::string>& filterNameAndDescription :
           filterNamesAndDescriptions) {
        const std::string& filterName = filterNameAndDescription.first;
        filterNames.push_back(filterName);
      }
      return filterNames;
    }

    template <class AFilter>
    auto UnionRecording<AFilter>::createVarSet(const std::string& name) const
    -> std::unique_ptr<AVarSet> {
      if (name.find("filter(") == 0 and name.rfind(")") == name.size() - 1)
      {
        B2INFO("Detected filter name");
        std::string filterName = name.substr(7, name.size() - 8);
        B2INFO("filterName = " << filterName);
        std::unique_ptr<AFilter> filter = m_filterFactory->create(filterName);
        if (not filter) {
          B2WARNING("Could not construct filter for name " << filterName);
          return std::unique_ptr<AVarSet>(nullptr);
        } else {
          AVarSet* filterVarSet = new FilterVarSet<AFilter>(filterName, std::move(filter));
          return std::unique_ptr<AVarSet>(filterVarSet);
        }
      } else {
        return std::unique_ptr<AVarSet>(nullptr);
      }
    }

    template <class AFilter>
    const std::vector<std::string>& UnionRecording<AFilter>::getVarSetNames() const
    {
      return m_param_varSetNames;
    }

    /* UnionRecordingFilter<> */
    template <class AFilterFactory>
    UnionRecordingFilter<AFilterFactory>::UnionRecordingFilter(
      const std::string& defaultRootFileName,
      const std::string& defaultTreeName)
      : Super(std::make_unique<AFilterFactory>(), defaultRootFileName, defaultTreeName)
    {
    }

    template <class AFilterFactory>
    UnionRecordingFilter<AFilterFactory>::~UnionRecordingFilter() = default;
  }
}
