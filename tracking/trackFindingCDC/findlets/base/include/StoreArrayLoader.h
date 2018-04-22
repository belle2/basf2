/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Helper findlet to allow combined findlets to import data from the DataStore.
     * This findlet can be used to retrieve data from a store array.
     * Please note that this module returns pointers.
     */
    template <class IOType>
    class StoreArrayLoader : public Findlet<IOType*> {

      /// Type of the base class
      using Super =  Findlet<IOType*>;

    public:
      /// Constructor taking the default name of the store vector which is the source for the import.
      StoreArrayLoader(const std::string& storeArrayName = "",
                       const std::string& classMnemomicName = "",
                       const std::string& classMnemomicDescription = "")
        : m_param_storeArrayName(storeArrayName),
          m_classMnemomicName(classMnemomicName),
          m_classMnemomicDescription(classMnemomicDescription)
      {
        if (m_classMnemomicDescription == "") {
          if (m_classMnemomicName != "") {
            m_classMnemomicDescription = m_classMnemomicName;
          } else {
            // Just a little bit of ADL, takem from StoreArraySwapper
            m_classMnemomicDescription = getClassMnemomicParameterDescription(static_cast<IOType*>(nullptr));
          }
        }

        if (m_classMnemomicName == "") {
          // Just a little bit of ADL, takem from StoreArraySwapper
          m_classMnemomicName = getClassMnemomicParameterName(static_cast<IOType*>(nullptr));
        }
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        std::string classMnemomicCapitalName = m_classMnemomicName;
        classMnemomicCapitalName[0] = ::toupper(classMnemomicCapitalName.at(0));

        moduleParamList->addParameter(prefixed(prefix, classMnemomicCapitalName + "sStoreArrayName"),
                                      m_param_storeArrayName,
                                      "Name of the StoreArray of the " + m_classMnemomicDescription,
                                      std::string(m_param_storeArrayName));
      }

      /// Short description of the findlet
      std::string getDescription() override
      {
        return "Swaps an interal vector of " + m_classMnemomicDescription + "s to the DataStore";
      }

      /// Receive signal before the start of the event processing
      void initialize() override
      {
        Super::initialize();
        StoreArray<IOType> storeArray(m_param_storeArrayName);
        storeArray.isRequired();
      }

      /// Loads the items from the DataStore
      void apply(std::vector<IOType*>& output) final {
        // Swap items to the DataStore
        StoreArray<IOType> storeArray(m_param_storeArrayName);

        output.reserve(output.size() + storeArray.getEntries());

        for (auto& item : storeArray)
        {
          output.push_back(&item);
        }
      }

    private:
      /// Name of the store vector to be used.
      std::string m_param_storeArrayName;

      /// Short name for the type of objects to be written out.
      std::string m_classMnemomicName;

      /// Short description for the type of objects to be written out.
      std::string m_classMnemomicDescription;
    };
  }
}
