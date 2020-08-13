/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/processModules/ProcessedEventsBackupList.h>

using namespace Belle2;

void ProcessedEventsBackupList::storeEvent(std::unique_ptr<EvtMessage> evtMsg, const StoreObjPtr<EventMetaData>& evtMetaData,
                                           const unsigned int workerId)
{
  EventMetaData eventMetaData(evtMetaData->getEvent(), evtMetaData->getRun(), evtMetaData->getExperiment());
  m_evtBackupVector.emplace_back(std::move(evtMsg), eventMetaData, workerId);
}

void ProcessedEventsBackupList::removeEvent(const EventMetaData& evtMetaData)
{
  const auto oldSize = m_evtBackupVector.size();

  m_evtBackupVector.erase(std::remove(m_evtBackupVector.begin(), m_evtBackupVector.end(), evtMetaData),
                          m_evtBackupVector.end());

  if (oldSize == m_evtBackupVector.size()) {
    B2WARNING("Event: " << evtMetaData.getEvent() << ", no matching event backup found in backup list");
  }
}

int ProcessedEventsBackupList::checkForTimeout(const Duration& timeout) const
{
  if (m_evtBackupVector.empty()) {
    return -1;
  }
  if (std::chrono::system_clock::now() - m_evtBackupVector[0].getTimestamp() > timeout) {
    return m_evtBackupVector[0].getWorkerId();
  } else {
    return -1;
  }
}

unsigned int ProcessedEventsBackupList::size() const
{
  return m_evtBackupVector.size();
}