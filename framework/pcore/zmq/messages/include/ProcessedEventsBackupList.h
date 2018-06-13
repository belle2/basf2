//
// Created by abaur on 26.05.18.
//

#pragma once

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>
#include <memory>
#include <chrono>


namespace Belle2 {


  // ##############################################
  //        ProcessedEventBackup
  // ##############################################

  class ProcessedEventBackup {
    /** Class to store the event backup data */
  public:
    ProcessedEventBackup(const std::unique_ptr<EvtMessage>& evtMsg) : m_eventMessageDataVec(evtMsg->buffer(),
          evtMsg->buffer() + evtMsg->size())
    {
    }

    /** If needed you can send the backup event again to a zmq socket */
    void sendToSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      const auto& message = ZMQMessageFactory::createMessage(m_eventMessageDataVec);
      message->toSocket(socket);    // send it across multicast
      B2WARNING("sent backup evt | size: " << m_eventMessageDataVec.size());
    }

  private:
    /** This char vector contains a copy of the whole event message data of the data store */
    const std::vector<char> m_eventMessageDataVec;
  };



  // ##############################################
  //        ProcessedEventsBackupList
  // ##############################################

  class ProcessedEventsBackupList {
  public:

    void storeEvt(const std::unique_ptr<EvtMessage>& evtMsg, UniqueEventId evtId)
    {
      m_evtBackupMap.emplace(evtId, ProcessedEventBackup(evtMsg));
      m_evtIdList.push_back(evtId);
    }


    void removeEvt(UniqueEventId evtId)
    {
      int vecPosition = getEvtIdListPosition(evtId);
      if (vecPosition >= 0) {
        auto mapPosition = m_evtBackupMap.find(evtId);
        B2ASSERT("Event backup map matches not with UniqueEventId list", mapPosition != m_evtBackupMap.end());
        B2DEBUG(100, "delete event " << mapPosition->first.getEvt());
        m_evtBackupMap.erase(mapPosition);
        m_evtIdList.erase(m_evtIdList.begin() + vecPosition);
      } else {B2ERROR("No element found... maybe event sent across multicast and parallel sent from worker to output then ignore this error a higher process timeout would avoid this");}
    }


    int checkForTimeout(std::chrono::duration<int, std::ratio<1, 1000>> timeout)
    {
      if (std::chrono::system_clock::now() - m_evtIdList[0].getTimestamp() > timeout) {
        return m_evtIdList[0].getWorker();
      } else {
        return -1;
      }
    }


    void sendWorkerEventsAndRemoveBackup(int worker, std::unique_ptr<ZMQSocket>& socket)
    {
      /*B2RESULT("Event Order: ");
      for(auto it=m_evtBackupMap.begin(); it!=m_evtBackupMap.end(); it++) {
        B2RESULT("Event: "<< it->first.getEvt());
      }*/
      for (auto it = m_evtBackupMap.begin(); it != m_evtBackupMap.end();) {
        if (it->first.getWorker() == worker) {
          B2DEBUG(100, "Delete event: " << " event: " << it->first.getEvt());
          it->second.sendToSocket(socket);;
          m_evtIdList.erase(m_evtIdList.begin() + getEvtIdListPosition(it->first));
          it = m_evtBackupMap.erase(it);
          B2DEBUG(100, "new backup list size: " << m_evtBackupMap.size());
        } else {
          it++;
        }
      }
    }


    int size()
    {
      return m_evtIdList.size();
    }


    ~ProcessedEventsBackupList()
    {
    }

  private:
    int getEvtIdListPosition(UniqueEventId evtId)
    {
      int position = -1;
      for (unsigned int i = 0; i < m_evtIdList.size(); i++) {
        if (m_evtIdList[i] == evtId) {
          position = i;
          break;
        }
      }
      return position; // returns -1 if event was not found in list
    }
    std::vector<UniqueEventId> m_evtIdList; // the event position in this list is the same as in the backupList
    std::map<UniqueEventId, ProcessedEventBackup>m_evtBackupMap;
  };
}
