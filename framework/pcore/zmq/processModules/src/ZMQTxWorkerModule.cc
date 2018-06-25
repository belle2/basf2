/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/processModules/ZMQTxWorkerModule.h>
#include <framework/core/Environment.h>

using namespace Belle2;

REG_MODULE(ZMQTxWorker)

ZMQTxWorkerModule::ZMQTxWorkerModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
           "set the number of processes to at least 1.",
           Environment::Instance().getNumberProcesses());
}

void ZMQTxWorkerModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamer.initialize(m_param_compressionLevel, m_param_handleMergeable);
      m_zmqClient.initialize(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, false);

      m_firstEvent = false;
    }

    const auto& message = m_streamer.stream();
    m_zmqClient.send(message);
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Tx worker event: " << ex.what());
    }
  }
}

void ZMQTxWorkerModule::terminate()
{
  m_zmqClient.terminate();
}
