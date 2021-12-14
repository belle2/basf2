/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/EventMerging/EventExtraInfoConverterModule.h>

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(EventExtraInfoConverter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  EventExtraInfoConverterModule::EventExtraInfoConverterModule() : Module()
  {
    setDescription("Convert the analysis object EventExtraInfo to the framework object MergedEventExtraInfo.");

    setPropertyFlags(c_ParallelProcessingCertified);
  }

  void EventExtraInfoConverterModule::initialize()
  {
    m_eventExtraInfo.isOptional();
    m_mergedEventExtraInfo.registerInDataStore(DataStore::c_DontWriteOut);
  }

  void EventExtraInfoConverterModule::event()
  {
    m_mergedEventExtraInfo.create();

    if (m_eventExtraInfo.isValid()) {
      for (std::string name : m_eventExtraInfo->getNames()) {
        m_mergedEventExtraInfo->addExtraInfo(name, m_eventExtraInfo->getExtraInfo(name));
      }
    }
  }
} // end Belle2 namespace

