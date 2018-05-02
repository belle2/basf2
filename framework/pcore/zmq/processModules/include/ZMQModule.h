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

  const std::string COM_PROTOCOL = "ipc://";
  const std::string COM_SOCK_PATH = "/tmp/";
  const std::string XPUB_NAME = "xpub";
  const std::string XSUB_NAME = "xsub";
  const std::string SOCK_XPUB_ADDR = COM_PROTOCOL + COM_SOCK_PATH + XPUB_NAME + ".socket";
  const std::string SOCK_XSUB_ADDR = COM_PROTOCOL + COM_SOCK_PATH + XSUB_NAME + ".socket";


  class ZMQModule : public Module {
  public:
    ZMQModule() : Module()
    {
      addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
      addParam("xpubSocketAddr", m_param_xpubProxySocket, "Address of the XPUB socket of the proxy");
      addParam("xsubSocketAddr", m_param_xsubProxySocket, "Address of the XSUB socket of the proxy");
      setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

      B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
               "set the number of processes to at least 1.", Environment::Instance().getNumberProcesses());

    }
    ~ZMQModule();

  protected:
    std::string m_param_socketName;
    std::string m_param_subscribeSocketName;
    std::string m_param_xpubProxySocket; //= SOCK_XPUB_ADDR;
    std::string m_param_xsubProxySocket; // = SOCK_XSUB_ADDR;

    int m_param_compressionLevel = 0;
    bool m_param_handleMergeable = true;

    bool m_firstEvent = true;

    StoreObjPtr<RandomGenerator> m_randomgenerator;

    std::unique_ptr<DataStoreStreamer> m_streamer;

    std::unique_ptr<zmq::socket_t> m_socket;
    std::unique_ptr<zmq::socket_t> m_subscribeSocket;
    std::unique_ptr<zmq::socket_t> m_pubSocket;
    std::unique_ptr<zmq::socket_t> m_subSocket;
    std::unique_ptr<zmq::context_t> m_context;

    void initializeObjects(bool bindToEndPoint);
    void initBroadcast();
    void subscribeBroadcast(const c_MessageTypes);
    virtual void proceedBroadcast() = 0;
    void sendBroadcast(c_MessageTypes msgType = c_MessageTypes::c_broadcastMessage, std::string* msgDataString = nullptr);

    unsigned int m_zmqTimeout = 1000;
    std::string m_uniqueID = "";

    virtual void createSocket() = 0;
  };

}
