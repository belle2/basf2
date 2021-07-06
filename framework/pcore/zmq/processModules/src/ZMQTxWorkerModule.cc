/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/processModules/ZMQTxWorkerModule.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
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
      m_zmqClient.initialize<ZMQ_PUSH>(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, false);

      m_firstEvent = false;
    }

    const auto& evtMessage = m_streamer.stream();
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_eventMessage, evtMessage);
    m_zmqClient.send(std::move(message));
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
