/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/processModules/ProcessedEventBackup.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <memory>
#include <chrono>


namespace Belle2 {
  /// List-like structure for storing and retaining event backups
  class ProcessedEventsBackupList {
    using Duration = std::chrono::milliseconds;

  public:
    /// Add a new event backup with the given information. Takes ownership of the evt message.
    void storeEvent(std::unique_ptr<EvtMessage> evtMsg, const StoreObjPtr<EventMetaData>& evtMetaData, const unsigned int workerId);

    /// Remove all backups with the given event meta data (on confirmation)
    void removeEvent(const EventMetaData& evtMetaData);

    /// Check the items for timeout. Returns -1 if no timeout happened and the worker id, if it did.
    int checkForTimeout(const Duration& timeout) const;

    /// Send all backups of a given worker directly to the multicast and delete them.
    template <class AZMQClient>
    void sendWorkerBackupEvents(unsigned int worker, const AZMQClient& socket);

    /// Check the size
    unsigned int size() const;

  private:
    std::vector<ProcessedEventBackup> m_evtBackupVector;
  };

  template <class AZMQClient>
  void ProcessedEventsBackupList::sendWorkerBackupEvents(unsigned int worker, const AZMQClient& socket)
  {
    for (auto it = m_evtBackupVector.begin(); it != m_evtBackupVector.end();) {
      if (it->getWorkerId() == worker) {
        B2DEBUG(100, "Delete event: " << it->getEventMetaData().getEvent());
        it->sendToSocket(socket);
        // TODO: better: delete backup on confirmation message
        m_evtBackupVector.erase(it);
        B2DEBUG(100, "new backup list size: " << m_evtBackupVector.size());
      } else {
        it++;
      }
    }
  }
}
