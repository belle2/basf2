/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>


namespace Belle2 {
  /**
   * Module connecting the worker path with the output path on the worker side.
   * Handles the data communication.
   */
  class ZMQTxWorkerModule : public Module {
  public:
    /// Constructor setting the moudle paramters
    ZMQTxWorkerModule();
    /// Pack the datastore and stream it.
    void event() override;
    /// Terminate the client and tell the monitor, we are done.
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
    ZMQClient m_zmqClient;
    /// The data store streamer
    StreamHelper m_streamer;
  };
}