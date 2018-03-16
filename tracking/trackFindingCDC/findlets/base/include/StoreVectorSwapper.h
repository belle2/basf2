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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <framework/core/ModuleParamList.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    /** Helper findlet to allow combined findlets to export more data to the DataStore.
     *  This is mainly to temporarily maintain the former behaviour of some finder parts
     *  that used to expose more of their internals. */
    template<class IOType, bool a_alwaysWrite = false>
    class StoreVectorSwapper:
      public Findlet<IOType> {

    private:
      /// Type of the base class
      using Super = Findlet<IOType>;

    public:
      /// Constructor taking the default name of the store vector which is the target of the swap.
      StoreVectorSwapper(const std::string& storeVectorName,
                         const std::string& classMnemomicName = "",
                         const std::string& classMnemomicDescription = "")
        : m_param_storeVectorName(storeVectorName),
          m_classMnemomicName(classMnemomicName),
          m_classMnemomicDescription(classMnemomicDescription)
      {
        if (m_classMnemomicDescription == "") {
          if (m_classMnemomicName != "") {
            m_classMnemomicDescription = m_classMnemomicName;
          } else {
            // Just a little bit of ADL
            m_classMnemomicDescription = getClassMnemomicParameterDescription(static_cast<IOType*>(nullptr));
          }
        }

        if (m_classMnemomicName == "") {
          // Just a little bit of ADL
          m_classMnemomicName = getClassMnemomicParameterName(static_cast<IOType*>(nullptr));
        }
      }

      /// Expose the parameters  to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        std::string classMnemomicCapitalName = m_classMnemomicName;
        classMnemomicCapitalName[0] = ::toupper(classMnemomicCapitalName.at(0));

        if (not a_alwaysWrite) {
          moduleParamList->addParameter(prefixed(prefix, "Write" + classMnemomicCapitalName + "s"),
                                        m_param_writeStoreVector,
                                        "Switch if " + m_classMnemomicDescription +
                                        "s shall be written to the DataStore",
                                        m_param_writeStoreVector);
        }

        moduleParamList->addParameter(prefixed(prefix, classMnemomicCapitalName + "sStoreObjName"),
                                      m_param_storeVectorName,
                                      "Name of the output StoreObjPtr of the " + m_classMnemomicDescription +
                                      "s generated within this module.",
                                      std::string(m_param_storeVectorName));
        //FIXME: Small parameter names
      }

      /// Short description of the findlet
      std::string getDescription() override
      {
        return "Swaps an interal vector of " + m_classMnemomicDescription + "s to the DataStore";
      }

    public:
      /// Receive signal before the start of the event processing
      void initialize() override
      {
        Super::initialize();
        if (m_param_writeStoreVector) {
          StoreWrappedObjPtr< std::vector<IOType> > storeVector(m_param_storeVectorName);
          storeVector.registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
        }
      }

      /// Receive signal for the start of a new event.
      void beginEvent() override
      {
        Super::beginEvent();

        /// Attain super cluster vector on the DataStore if needed.
        if (m_param_writeStoreVector) {
          StoreWrappedObjPtr< std::vector<IOType> > storeVector(m_param_storeVectorName);
          if (not storeVector.isValid()) {
            storeVector.construct();
          }
        }
        m_backup.clear();
      }

      /// Swaps the items to the DataStore or to the backup storage location.
      void apply(std::vector<IOType>& input) final {
        // Swap items to the DataStore
        if (m_param_writeStoreVector)
        {
          StoreWrappedObjPtr< std::vector<IOType> > storeVector(m_param_storeVectorName);
          std::vector<IOType>& sink = *storeVector;
          sink.swap(input);
        } else {
          m_backup.swap(input);
        }
      }

    private:
      /// Name of the store vector to be used.
      std::string m_param_storeVectorName;

      /// Switch if the store vector should be filled.
      bool m_param_writeStoreVector = a_alwaysWrite;

      /// Short name for the type of objects to be written out.
      std::string m_classMnemomicName;

      /// Short description for the type of objects to be written out.
      std::string m_classMnemomicDescription;

      /// Backup storage if the vector should not be written to the DataStore
      std::vector<IOType> m_backup;
    };
  }
}
