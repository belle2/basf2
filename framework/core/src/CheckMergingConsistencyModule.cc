/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/CheckMergingConsistencyModule.h>

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CheckMergingConsistency)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  CheckMergingConsistencyModule::CheckMergingConsistencyModule() : Module()
  {
    setDescription("This module checks if events that will be merged are consistent (otherwise events will be skipped later).");

    setPropertyFlags(c_ParallelProcessingCertified);
  }

  CheckMergingConsistencyModule::~CheckMergingConsistencyModule() = default;

  void CheckMergingConsistencyModule::init(const std::string& option, bool eventMixing)
  {
    m_option = option;
    m_eventMixing = eventMixing;
  }

  void CheckMergingConsistencyModule::initialize()
  {
    m_mergedEventConsistency.registerInDataStore(DataStore::c_DontWriteOut);
    if (!m_EventExtraInfo_main.isOptional() or !m_EventExtraInfo_indep.isOptional()) {
      B2WARNING("EventExtraInfo does not exist in both paths. Cannot check events for consistency.");
    }

    if (!m_option.empty() && m_option != "charge") {
      B2ERROR("Only available options for consistency check are 'charge' or no check at all.");
    }
  }

  void CheckMergingConsistencyModule::event()
  {
    m_mergedEventConsistency.create();
    m_mergedEventConsistency->setConsistent(false);

    if (m_option == "charge") {
      if (m_EventExtraInfo_main.isValid() && m_EventExtraInfo_indep.isValid()) {
        if (!m_EventExtraInfo_main->hasExtraInfo("charge") || !m_EventExtraInfo_indep->hasExtraInfo("charge")) {
          B2ERROR("No EventExtraInfo 'charge' found.");
          return;
        }

        int charge_main = m_EventExtraInfo_main->getExtraInfo("charge");
        int charge_indep = m_EventExtraInfo_indep->getExtraInfo("charge");
        if (m_eventMixing) {
          // charge of both removed tags has to be opposite
          if (charge_main == -charge_indep) {
            m_mergedEventConsistency->setConsistent(true);
          }
        } else {
          // charge of embedded object has to correspond to charge of removed tag
          if (charge_main == charge_indep) {
            m_mergedEventConsistency->setConsistent(true);
          }
        }
      } else {
        // if the information is not there, we issued a warning in the initialization that we will just go on
        m_mergedEventConsistency->setConsistent(true);
      }
    } else {
      // default behavior: just set flag to true
      m_mergedEventConsistency->setConsistent(true);
    }

  }
} // end Belle2 namespace

