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
  /**
   * Module connecting the input path with the worker path on the input side.
   * Handles the data communication and the event backup. Tells the monitor to kill processes if needed and sends an end message when done.
   */
  class ZMQTxInputModule : public Module {
  public:
    /// Constructor setting the moudle paramters
    ZMQTxInputModule();
    /// Pack the datastore and send it. Also handle ready or hello messages of workers.
    void event() override;
    /// Terminate the client and tell the monitor, we are done. Tell the output to end if all backups are out.
    void terminate() override;

  private:
    /// The list of next worker ids.
    std::deque<unsigned int> m_nextWorker;
    /// The list of all workers (to say goodbye properly).
    std::vector<unsigned int> m_workers;
    /// The backup list
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

    /// Check if a worker has fallen into a timeout and send a kill message if needed.
    void checkWorkerProcTimeout();
  };
}