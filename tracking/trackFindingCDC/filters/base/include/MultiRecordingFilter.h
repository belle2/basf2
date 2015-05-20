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

#include "RecordingFilter.h"

#include <tracking/trackFindingCDC/varsets/MultiVarSet.h>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class Object_>
    class MultiRecordingFilter: public RecordingFilter<MultiVarSet<Object_>> {

    private:
      /// Type of the super class
      typedef RecordingFilter<MultiVarSet<Object_>> Super;

    public:
      /// Type of the object to be analysed.
      typedef Object_ Object;

    public:
      /// Constructor of the filter.
      MultiRecordingFilter(const std::string& defaultRootFileName = "records.root",
                           const std::string& defaultTreeName = "records") :
        Super(defaultRootFileName, defaultTreeName)
      {;}

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        MultiVarSet<Object>& multiVarSet = Super::getVarSet();

        /// Create and add the concrete varsets from the varset parameter.
        for (std::string name : getVarSetNames()) {
          std::unique_ptr<BaseVarSet<Object>> varSet = createVarSet(name);
          if (varSet) {
            multiVarSet.push_back(std::move(varSet));
          } else {
            B2WARNING("Could not create a variable set from name " << name);
          }
        }
        Super::initialize();
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        Super::terminate();
        MultiVarSet<Object>& multiVarSet = Super::getVarSet();
        multiVarSet.clear();
      }

      /** Set the parameter with key to values.
       *
       *  Parameters are:
       *  root_file_name = Name of the ROOT file to be written.
       *  tree_name -  Name of the Tree to be written.
       *  returned_cell_weight - CellWeight this filter should return when called. Defaults to NOT_A_CELL
       *  varsets - Comma separated list of names refering to concrete variable sets.
       */
      virtual void setParameter(const std::string& key, const std::string& value)
      {
        if (key == "varsets") {
          m_commaSeparatedVarSetNames = value;
          B2INFO("Filter received parameter 'varsets' = " << m_commaSeparatedVarSetNames);
        } else {
          Super::setParameter(key, value);
        }
      }

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual std::map<std::string, std::string> getParameterDescription()
      {
        std::map<std::string, std::string> des = Super::getParameterDescription();
        des["varset"] = "Comma separated list of names refering to concrete variable sets.";
        return des;
      }

      /// Getter for the list of valid names of concret variable sets.
      virtual std::vector<std::string> getValidVarSetNames() const
      { return std::vector<std::string> (); }

      /// Create a variable set for the given name.
      virtual std::unique_ptr<BaseVarSet<Object>> createVarSet(const std::string& /*name*/) const
      { return std::unique_ptr<BaseVarSet<Object>>(nullptr); }

      /// Splits the comma separated variable names list into a vector of names.
      std::vector<std::string> getVarSetNames() const
      {
        std::vector<std::string> varSetNames;
        boost::split(varSetNames, m_commaSeparatedVarSetNames, boost::is_any_of(","));
        // Remove leading and trailing white space.
        for (std::string& varSetName : varSetNames) {
          boost::trim(varSetName);
        }
        return varSetNames;
      }

    private:
      /// A string containing the comma separated names of concrete variable sets to be recorded.
      std::string m_commaSeparatedVarSetNames;
    };
  }
}
