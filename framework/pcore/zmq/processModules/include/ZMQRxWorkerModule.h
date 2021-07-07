/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>

namespace Belle2 {
  /**
   * Module connecting the input path with the worker path on the worker side.
   * Handles the data communication and the ready/hello messages to the input process.
   */
  class ZMQRxWorkerModule : public Module {
  public:
    /// Constructor setting the moudle paramters
    ZMQRxWorkerModule();
    /// Receive an event and store it in the datastore. Tell the input process we are ready.
    void event() override;
    /// Initialize the streamer
    void initialize() override;
    /// Terminate the client and tell the monitor, we are done.
    void terminate() override;

  private:
    /// How many events do we want to have in the buffer
    unsigned int m_param_bufferSize;
    /// Maximal time to wait in polling
    unsigned int m_param_maximalWaitingTime;

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
  };
}
