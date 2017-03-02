/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <TObject.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Storable object for the variables calculated in the SoftwareTriggerModule. The module
     * will write out instances of this class to the data store if you turn on the debug feature.
     *
     * This class is more or less only a wrapper around an std::map<string, double> where the strings
     * are the identifier of the variables (prefixed with the base_identifier) and the double are the
     * variables' values.
     */
    class SoftwareTriggerVariables : public TObject {
    public:
      /// Append a new variable to the collection. Make sure not to override existing ones.
      void append(const std::string& identifier, const double& value)
      {
        m_results[identifier] = value;
      }

      /// Get the whole map of results for analysis.
      const std::map<std::string, double>& get() const
      {
        return m_results;
      }

      /// Clear all results.
      void clear()
      {
        m_results.clear();
      }

    private:
      /// Internal storage of the variables with names.
      std::map<std::string, double> m_results;

      /** Making this class a ROOT class.*/
      ClassDef(SoftwareTriggerVariables, 1);
    };
  }
}
