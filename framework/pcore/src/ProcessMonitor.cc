/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Baur, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/ProcessMonitor.h>
#include <framework/core/EventProcessor.h>
#include <framework/logging/LogMethod.h>
#include <framework/pcore/ProcHandler.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>


#include <thread>
#include <signal.h>

using namespace Belle2;

void ProcessMonitor::subscribe(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                               const std::string& controlSocketAddress)
{
  m_context = std::make_unique<zmq::context_t>(1);

  if (not ProcHandler::startProxyProcess()) {
    m_controlSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
    m_controlSocket->bind(controlSocketAddress);
    m_controlSocket->setsockopt(ZMQ_LINGER, 0);

    // Time to setup the proxy
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
    m_pubSocket->connect(pubSocketAddress);
    m_pubSocket->setsockopt(ZMQ_LINGER, 0);

    m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);
    m_subSocket->connect(subSocketAddress);
    m_subSocket->setsockopt(ZMQ_LINGER, 0);
  } else {
    // The default will be to not do anything on signals...
    EventProcessor::installMainSignalHandlers(SIG_IGN);

    // We open a new context here in the new process
    m_context = std::make_unique<zmq::context_t>(1);

    m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_XPUB);
    // ATTENTION: this is switched on intention!
    m_pubSocket->bind(subSocketAddress);
    m_pubSocket->setsockopt(ZMQ_LINGER, 0);

    m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_XSUB);
    // ATTENTION: this is switched on intention!
    m_subSocket->bind(pubSocketAddress);
    m_subSocket->setsockopt(ZMQ_LINGER, 0);

    m_controlSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);
    m_controlSocket->connect(controlSocketAddress);
    m_controlSocket->setsockopt(ZMQ_LINGER, 0);
    m_controlSocket->setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // Wait until control socket has started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    B2DEBUG(10, "Will now start the proxy..");
    zmq::proxy_steerable(*m_pubSocket, *m_subSocket, nullptr, *m_controlSocket);
    B2DEBUG(10, "Proxy has finished");
    terminate();
    exit(0);
  }

  const char helloMessages = static_cast<char>(c_MessageTypes::c_helloMessage);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &helloMessages, 1);
  const char deathMessages = static_cast<char>(c_MessageTypes::c_deathMessage);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &deathMessages, 1);
  const char terminateMessages = static_cast<char>(c_MessageTypes::c_terminateMessage);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &terminateMessages, 1);

  B2DEBUG(10, "Started multicast publishing on " << pubSocketAddress << " and subscribing on " << subSocketAddress);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void ProcessMonitor::terminate()
{
  if (m_subSocket) {
    m_subSocket->close();
    m_subSocket.release();
  }
  if (m_pubSocket) {
    m_pubSocket->close();
    m_pubSocket.release();
  }
  if (m_controlSocket) {
    m_controlSocket->close();
    m_controlSocket.release();
  }
  if (m_context) {
    m_context->close();
    m_context.release();
  }
}

void ProcessMonitor::reset()
{
  m_subSocket.release();
  m_pubSocket.release();
  m_controlSocket.release();
  m_context.release();
}

void ProcessMonitor::killProcesses(unsigned int timeout)
{
  B2ASSERT("Only the monitoring process is allowed to kill processes", ProcHandler::isProcess(ProcType::c_Monitor)
           or ProcHandler::isProcess(ProcType::c_Init));


  if (not m_processList.empty()) {
    // Try to kill them gently...
    const auto& pcbMulticastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_stopMessage);
    pcbMulticastMessage->toSocket(m_pubSocket);

    // Give them some time
    // TODO: we can have a better timeout here!
    std::this_thread::sleep_for(std::chrono::seconds(timeout));
  }

  // Kill the proxy and give it some time to terminate
  if (m_controlSocket) {
    m_controlSocket->send(ZMQMessageHelper::createZMQMessage("TERMINATE"));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // If everything did not help, we will kill all of them
  ProcHandler::killAllProcesses();
}

void ProcessMonitor::waitForRunningInput(const int timeout)
{
  if (processesWithType(ProcType::c_Input) < 1) {
    checkMulticast(timeout);
    if (processesWithType(ProcType::c_Input) < 1) {
      B2FATAL("Input process did not start properly!");
    };
  }
}

void ProcessMonitor::waitForRunningWorker(const int timeout)
{
  if (processesWithType(ProcType::c_Worker) < m_requestedNumberOfWorkers) {
    checkMulticast(timeout);
    if (processesWithType(ProcType::c_Worker) < m_requestedNumberOfWorkers) {
      B2FATAL("Some Worker processes did not start properly!");
    };
  }
}


void ProcessMonitor::waitForRunningOutput(const int timeout)
{
  if (processesWithType(ProcType::c_Output) < 1) {
    checkMulticast(timeout);
    if (processesWithType(ProcType::c_Output) < 1) {
      B2FATAL("Output process did not start properly!");
    };
  }
}

void ProcessMonitor::initialize(unsigned int requestedNumberOfWorkers)
{
  m_requestedNumberOfWorkers = requestedNumberOfWorkers;
}

void ProcessMonitor::checkMulticast(const int timeout)
{
  while (true) {
    if (not ZMQHelper::pollSocket(m_subSocket, timeout * 1000)) {
      return;
    }
    const auto& pcbMulticastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (pcbMulticastMessage->isMessage(c_MessageTypes::c_helloMessage)) {
      const int pid = std::stoi(pcbMulticastMessage->getData());
      const ProcType procType = ProcHandler::getProcType(pid);
      m_processList[pid] = procType;
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Input) << " input processes.");
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Output) << " output processes.");
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Worker) << " worker processes.");
    } else if (pcbMulticastMessage->isMessage(c_MessageTypes::c_terminateMessage)) {
      const int pid = std::stoi(pcbMulticastMessage->getData());
      const auto& processIt = m_processList.find(pid);
      if (processIt == m_processList.end()) {
        B2WARNING("An unknown PID died!");
        continue;
      }
      const ProcType procType = processIt->second;
      if (procType == ProcType::c_Worker) {
        m_requestedNumberOfWorkers--;
        B2DEBUG(10, "Now we will only need " << m_requestedNumberOfWorkers << " of workers anymore");
      }
      processIt->second = ProcType::c_Stopped;
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Input) << " input processes.");
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Output) << " output processes.");
      B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Worker) << " worker processes.");
    } else if (pcbMulticastMessage->isMessage(c_MessageTypes::c_deathMessage)) {
      const int workerPID = atoi(pcbMulticastMessage->getData().c_str());
      B2DEBUG(10, "Got message to kill worker " << workerPID);
      if (kill(workerPID, SIGKILL) == 0) {
        B2WARNING("Needed to kill worker  " << workerPID << " as it was requested.");
      } else {
        B2ERROR("Try to kill process " << workerPID << ", but process is already gone.");
      }
      // TODO: Do we want to decrease the number of workers here or later in the check of the processes?
    }
  }
}

void ProcessMonitor::checkChildProcesses()
{
  // Copy is intended, as we do not want the signal handler to change our list
  std::vector<int> currentProcessList = ProcHandler::getPIDList();
  // Check for processes, which where there last time but are gone now (so they died)
  for (auto iter = m_processList.begin(); iter != m_processList.end();) {
    const auto& pair = *iter;

    B2ASSERT("This pid should not be in our list!", pair.first != 0);
    //once a process is gone from the global list, remove them from our own, too.
    if (std::find(currentProcessList.begin(), currentProcessList.end(), pair.first) != currentProcessList.end()) {
      ++iter;
      continue;
    }

    // if the process has gone down properly, it should now be set to "Stopped"
    if (pair.second == ProcType::c_Input) {
      B2FATAL("An input process has died unexpected! Need to go down.");
    } else if (pair.second == ProcType::c_Output) {
      B2FATAL("An output process has died unexpected! Need to go down.");
    } else if (pair.second == ProcType::c_Proxy) {
      B2FATAL("A proxy process has died unexpected! Need to go down.");
    } else if (pair.second == ProcType::c_Worker) {
      B2WARNING("A worker process has died unexpected. The events should be save. Will try to restart the worker...");
      B2ASSERT("A worker died but none was present?", processesWithType(ProcType::c_Worker) != 0);
    } else if (pair.second == ProcType::c_Stopped) {
      B2DEBUG(10, "An children process has died expectedly.");
    }

    iter = m_processList.erase(iter);
  }

  if (m_processList.empty()) {
    m_hasEnded = true;
  }
}

void ProcessMonitor::checkSignals(int g_signalReceived)
{
  if (g_signalReceived > 0) {
    B2DEBUG(10, "Received signal " << g_signalReceived);
    m_hasEnded = true;
  }
}

bool ProcessMonitor::hasEnded()
{
  return m_hasEnded;
}

unsigned int ProcessMonitor::needMoreWorkers()
{
  const int neededWorkers = m_requestedNumberOfWorkers - processesWithType(ProcType::c_Worker);
  if (neededWorkers < 0) {
    B2FATAL("Something went completely wrong here! I have more workers as requested...");
  }
  if (neededWorkers > 0) {
    B2DEBUG(10, "I need to restart " << neededWorkers << " workers");
  }
  return static_cast<unsigned int>(neededWorkers);
}

unsigned int ProcessMonitor::processesWithType(const ProcType& procType) const
{
  auto correctProcType = [&procType](const auto & pair) {
    return pair.second == procType;
  };
  return std::count_if(m_processList.begin(), m_processList.end(), correctProcType);
}