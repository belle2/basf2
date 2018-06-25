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

#include <framework/core/Module.h>

#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>


namespace Belle2 {
  class ZMQTxWorkerModule : public Module {
  public:
    /// Add the parameters
    ZMQTxWorkerModule();

    /// Call the event function
    void event() override;

    /// Call the terminate function
    void terminate() override;

  private:
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
    ZMQClient<ZMQ_PUSH> m_zmqClient;
    /// The data store streamer
    StreamHelper m_streamer;
  };
}