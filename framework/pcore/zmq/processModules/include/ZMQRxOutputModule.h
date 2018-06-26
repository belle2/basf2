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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>

#include <memory>

namespace Belle2 {
  class ZMQRxOutputModule : public Module {
  public:
    ZMQRxOutputModule();
    void event() override;
    void initialize() override;
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
    ZMQClient m_zmqClient;
    /// The data store streamer
    StreamHelper m_streamer;

    /// The event meta data in the data store needed for confirming events
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /// The random generator in the data store
    StoreObjPtr<RandomGenerator> m_randomgenerator;
  };
}