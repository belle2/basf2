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
     */
    class SoftwareTriggerVariables : public TObject {
    public:
      /// Add a new cut result to the storage or override the result with the same name.
      void append(const std::string& identifier, const double& value)
      {
        m_results[identifier] = value;
      }

      /// Clear all results
      void clear()
      {
        m_results.clear();
      }

    private:
      /// Internal storage of the cut decisions with names.
      std::map<std::string, double> m_results;

      /** Making this class a ROOT class.*/
      ClassDef(SoftwareTriggerVariables, 1);
    };
  }
}
