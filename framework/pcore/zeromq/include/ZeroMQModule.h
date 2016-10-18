#pragma once

#include <framework/core/Module.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <zmq.hpp>

#include <memory.h>

namespace Belle2 {
  class ZeroMQModule : public Module {
  public:
    ZeroMQModule() : Module()
    {
      addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
      setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

      B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
               "set the number of processes to at least 1.", Environment::Instance().getNumberProcesses());
    }

  protected:
    std::string m_param_socketName;

    int m_param_compressionLevel = 0;
    bool m_param_handleMergeable = true;

    bool m_firstEvent = true;

    StoreObjPtr<RandomGenerator> m_randomgenerator;

    std::unique_ptr<DataStoreStreamer> m_streamer;

    std::unique_ptr<zmq::socket_t> m_socket;
    std::unique_ptr<zmq::context_t> m_context;

    void initializeObjects(bool bindToEndPoint);

    virtual void createSocket() = 0;
  };
}