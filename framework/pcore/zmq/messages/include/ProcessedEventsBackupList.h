//
// Created by abaur on 26.05.18.
//

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <memory>
#include <chrono>
#include <framework/pcore/zmq/messages/ProcessedEventsBackupList.h>


namespace Belle2 {


  // ##############################################
  //        ProcessedEventBackup
  // ##############################################

  class ProcessedEventBackup {
    /** Class to store the event backup data */
  public:
    ProcessedEventBackup(const std::unique_ptr<EvtMessage>& evtMsg, EventMetaData& evtMetaData,
                         unsigned int workerId) : m_eventMessageDataVec(evtMsg->buffer(),
                               evtMsg->buffer() + evtMsg->size()), m_eventMetaData(evtMetaData), m_workerId(workerId)
    {
    }

    /** If needed you can send the backup event again to a zmq socket */
    void sendToSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      const auto& message = ZMQMessageFactory::createMessage(m_eventMessageDataVec);
      message->toSocket(socket);    // send it across multicast
      B2WARNING("sent backup evt: " << m_eventMetaData.getEvent() << " | size: " << m_eventMessageDataVec.size());
    }

    auto& getEventMetaData() const {return m_eventMetaData;}
    const auto& getTimestamp() const {return m_timestamp;}
    const auto& getWorkerId() const {return m_workerId;}

    bool operator== (const ProcessedEventBackup& processedEventBackup)
    {
      return m_eventMetaData == processedEventBackup.getEventMetaData();
    }

  private:
    /** This char vector contains a copy of the whole event message data of the data store */
    std::vector<char> m_eventMessageDataVec;
    EventMetaData m_eventMetaData;
    unsigned int m_workerId;
    std::chrono::time_point<std::chrono::system_clock> m_timestamp = std::chrono::system_clock::now();
  };






  // ##############################################
  //        ProcessedEventsBackupList
  // ##############################################

  class ProcessedEventsBackupList {
  public:

    void storeEvt(const std::unique_ptr<EvtMessage>& evtMsg, const StoreObjPtr<EventMetaData>& evtMetaData, const unsigned int workerId)
    {
      EventMetaData eventMetaData(evtMetaData->getEvent(), evtMetaData->getRun(), evtMetaData->getExperiment());
      ProcessedEventBackup processedEventBackup(evtMsg, eventMetaData, workerId);
      m_evtBackupVector.emplace_back(processedEventBackup);
    }


    void removeEvt(const EventMetaData& evtMetaData)
    {
      const auto compareEvtMetaData = [&](const auto & item) {
        return item.getEventMetaData() == evtMetaData;
      };
      auto oldSize = m_evtBackupVector.size();
      m_evtBackupVector.erase(std::remove_if(m_evtBackupVector.begin(), m_evtBackupVector.end(), compareEvtMetaData),
                              m_evtBackupVector.end());
      if (oldSize == m_evtBackupVector.size()) {
        B2ERROR("No matching event backup found in backup list");
      }
    }


    int checkForTimeout(std::chrono::duration<int, std::ratio<1, 1000>> timeout)
    {
      if (std::chrono::system_clock::now() - m_evtBackupVector[0].getTimestamp() > timeout) {
        return m_evtBackupVector[0].getWorkerId();
      } else {
        return -1;
      }
    }


    void sendWorkerBackupEvents(unsigned int worker, std::unique_ptr<ZMQSocket>& socket)
    {
      for (auto it = m_evtBackupVector.begin(); it != m_evtBackupVector.end();) {
        if (it->getWorkerId() == worker) {
          B2DEBUG(100, "Delete event: " << it->getEventMetaData().getEvent());
          it->sendToSocket(socket);
          //better: delete backup with confirmation message
          m_evtBackupVector.erase(it);
          B2DEBUG(100, "new backup list size: " << m_evtBackupVector.size());
        } else {
          it++;
        }
      }
    }

    unsigned int size()
    {
      return m_evtBackupVector.size();
    }

    std::vector<ProcessedEventBackup> m_evtBackupVector;
  };
}
