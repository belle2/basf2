#pragma once

#include <framework/core/Module.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

#include <zmq.hpp>

#include <memory.h>
#include <chrono>
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

      m_pollSocketPtrList.reserve(2);
      m_workerProcTimeout =  std::chrono::duration<int, std::ratio<1, 1000>>(6000);
    }

  protected:
    std::string m_uniqueID = "";

    std::string m_param_socketName;
    std::string m_param_xpubProxySocketName;
    std::string m_param_xsubProxySocketName;

    int m_param_compressionLevel = 0;
    bool m_param_handleMergeable = true;

    bool m_firstEvent = true;
    int m_helloMulticastDelay = 1;
    int m_pollTimeout = 20 * 1000; //timeout for poll in ms
    std::chrono::duration<int, std::ratio<1, 1000>> m_workerProcTimeout; //(1000); //ms

    StoreObjPtr<RandomGenerator> m_randomgenerator;

    std::unique_ptr<DataStoreStreamer> m_streamer;

    // TODO: rename m_socket to m_eventSocket or m_dataSocket
    std::unique_ptr<zmq::socket_t> m_socket = nullptr;
    std::unique_ptr<zmq::socket_t> m_pubSocket = nullptr;
    std::unique_ptr<zmq::socket_t> m_subSocket = nullptr;
    std::unique_ptr<zmq::context_t> m_context = nullptr;

    std::vector<zmq::socket_t*> m_pollSocketPtrList; // this vector becomes it items in initializeObjects() when the sockets are created
    // for this constants you have to know the order of the items in the m_pollSocketPtrList
    const int c_subSocket = 1;  // 00001
    const int c_socket = 2;     // 00010

    virtual void initializeObjects(bool bindToEndPoint);
    void initMulticast();
    void subscribeMulticast(const c_MessageTypes);
    virtual void proceedMulticast() = 0;
    virtual void createSocket() = 0;
  };

}
