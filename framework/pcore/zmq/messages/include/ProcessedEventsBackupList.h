//
// Created by abaur on 26.05.18.
//

#pragma once

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>
#include <framework/pcore/zmq/messages/EventMessageBuffer.h>
#include <memory>


namespace Belle2 {


  // Object to store the backup data
  class ProcessedEventBackup {
  public:
    ProcessedEventBackup(char* eventData, int size)
    {
      m_eventMessageData = new char[size];
      memcpy(m_eventMessageData, eventData, size);
      m_eventMessageSize = size;
    }


    void sendToSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      EventMessageBuffer evtMsgBuffer(m_eventMessageData, m_eventMessageSize);
      const auto& message = ZMQMessageFactory::createMessage(evtMsgBuffer); // its type is automatic set as event Message
      message->toSocket(socket);    // send it across multicast
    }


    void deleteEventData()
    {
      if (m_eventMessageSize > 0) {
        delete[] m_eventMessageData;
        m_eventMessageSize = 0;
      }
    }

    ~ProcessedEventBackup()
    {
    }


  private:
    char* m_eventMessageData;
    int m_eventMessageSize;
  };



  // ##############################################
  //        ProcessedEventsBackupList
  // ##############################################

  class ProcessedEventsBackupList {
  public:

    void storeEvt(char* evtData, int size, UniqueEventId evtId)
    {
      m_evtBackupMap.emplace(evtId, ProcessedEventBackup(evtData, size));
      m_evtIdList.push_back(evtId);
    }


    void removeEvt(UniqueEventId evtId)
    {
      int vecPosition = getEvtIdListPosition(evtId);
      if (vecPosition >= 0) {
        auto mapPosition = m_evtBackupMap.find(evtId);
        B2ASSERT("Event backup map matches not with UniqueEventId list", mapPosition != m_evtBackupMap.end());
        B2DEBUG(100, "delete event " << mapPosition->first.getEvt());
        mapPosition->second.deleteEventData();
        m_evtBackupMap.erase(mapPosition);
        m_evtIdList.erase(m_evtIdList.begin() + vecPosition);
      } else {B2ERROR("No element found... maybe event sent across multicast and parallel sent from worker to output ");}
    }


    int checkForTimeout(int timeout)
    {
      if (difftime(time(NULL), m_evtIdList[0].getTimestamp()) > timeout) {
        return m_evtIdList[0].getWorker();
      } else
        return -1;
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
          it->second.sendToSocket(socket);
          it->second.deleteEventData();
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
      for (auto it = m_evtBackupMap.begin(); it != m_evtBackupMap.end(); it++) {
        it->second.deleteEventData();
      }
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