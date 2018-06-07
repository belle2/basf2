#pragma once

#include <framework/pcore/EvtMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/zmq/processModules/ZMQTxModule.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ProcessedEventsBackupList.h>
#include <deque>
#include <zmq.hpp>



namespace Belle2 {
  class ZMQTxInputModule : public ZMQTxModule {
  public:
    //ZMQTxInputModule() :
    //  ZMQTxModule(){}

    void event() override;
    void terminate() override;

  protected:
    virtual std::unique_ptr<ZMQIdMessage> readEventToMessage(std::string& NextWorkerID);
    std::unique_ptr<EvtMessage> m_eventMessage;

  private:
    std::deque<unsigned int> m_nextWorker;
    std::vector<unsigned int> m_workers;
    StoreObjPtr<EventMetaData> m_eventMetaData;

    ProcessedEventsBackupList m_procEvtBackupList;

    void createSocket() override;
    void proceedMulticast() override;
    unsigned int getNextWorker();
    void getWorkersReadyMessages();
    int checkWorkerProcTimeout();




  };
}