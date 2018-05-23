#pragma once

#include <framework/core/Module.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

#include <zmq.hpp>

#include <memory.h>

#include <iostream>


namespace Belle2 {

  class ZMQModule : public Module {
  public:
    ZMQModule() : Module()
    {
      addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
      addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
      addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
      setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

      B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
               "set the number of processes to at least 1.", Environment::Instance().getNumberProcesses());

    }
    ~ZMQModule();

  protected:
    std::string m_param_socketName;
    std::string m_param_xpubProxySocketName;
    std::string m_param_xsubProxySocketName;

    int m_param_compressionLevel = 0;
    bool m_param_handleMergeable = true;

    bool m_firstEvent = true;

    StoreObjPtr<RandomGenerator> m_randomgenerator;

    std::unique_ptr<DataStoreStreamer> m_streamer;

    std::unique_ptr<zmq::socket_t> m_socket = nullptr;
    std::unique_ptr<zmq::socket_t> m_pubSocket = nullptr;
    std::unique_ptr<zmq::socket_t> m_subSocket = nullptr;
    std::unique_ptr<zmq::context_t> m_context = nullptr;

    virtual void initializeObjects(bool bindToEndPoint);
    void initMulticast();
    void subscribeMulticast(const c_MessageTypes);
    virtual void proceedMulticast() = 0;
    void sendMulticast(c_MessageTypes msgType = c_MessageTypes::c_multicastMessage, std::string* msgDataString = nullptr);
    int waitForStartEvtProc();

    unsigned int m_helloMulticastDelay = 1; // time in seconds to wait sending c_helloMessage, this is necessary why ever
    unsigned int m_zmqTimeout = 1000;
    std::string m_uniqueID = "";

    virtual void createSocket() = 0;
  };

}
