#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>

#include <framework/pcore/zmq/processModules/ProcessedEventsBackupList.h>
#include <deque>

namespace Belle2 {
  class ZMQTxInputModule : public Module {
  public:
    ZMQTxInputModule();
    void event() override;
    void terminate() override;

  private:
    std::deque<unsigned int> m_nextWorker;
    std::vector<unsigned int> m_workers;

    ProcessedEventsBackupList m_procEvtBackupList;

    /// Flag to use the event backup or not.
    bool m_param_useEventBackup = true;

    /// Set to false if the objects are initialized
    bool m_firstEvent = true;

    /// Parameter: name of the data socket
    std::string m_param_socketName;
    /// Parameter: name of the pub multicast socket
    std::string m_param_xpubProxySocketName;
    /// Parameter: name of the sub multicast socket
    std::string m_param_xsubProxySocketName;
    /// Parameter: Compression level of the streamer
    int m_param_compressionLevel = 0;
    /// Parameter: Can we handle mergeables?
    bool m_param_handleMergeable = true;

    /// Our ZMQ client
    ZMQClient m_zmqClient;
    /// The data store streamer
    StreamHelper m_streamer;

    /// The event meta data in the data store needed for confirming events
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /// The random generator in the data store
    StoreObjPtr<RandomGenerator> m_randomgenerator;

    void checkWorkerProcTimeout();
  };
}