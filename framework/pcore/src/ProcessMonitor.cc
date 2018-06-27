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
#include <framework/pcore/GlobalProcHandler.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>

#include <thread>
#include <signal.h>

using namespace Belle2;

void ProcessMonitor::subscribe(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                               const std::string& controlSocketAddress)
{
  if (GlobalProcHandler::startProxyProcess()) {
    m_client.reset();

    // The default will be to not do anything on signals...
    EventProcessor::installMainSignalHandlers(SIG_IGN);

    // We open a new context here in the new process
    zmq::context_t context(1);

    zmq::socket_t pubSocket(context, ZMQ_XPUB);
    // ATTENTION: this is switched on intention!
    pubSocket.bind(subSocketAddress);
    pubSocket.setsockopt(ZMQ_LINGER, 0);

    zmq::socket_t subSocket(context, ZMQ_XSUB);
    // ATTENTION: this is switched on intention!
    subSocket.bind(pubSocketAddress);
    subSocket.setsockopt(ZMQ_LINGER, 0);

    zmq::socket_t controlSocket(context, ZMQ_SUB);
    controlSocket.bind(controlSocketAddress);
    controlSocket.setsockopt(ZMQ_LINGER, 0);
    controlSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    B2DEBUG(10, "Will now start the proxy..");
    bool running = true;
    while (running) {
      try {
        zmq::proxy_steerable(pubSocket, subSocket, nullptr, controlSocket);
        running = false;
      } catch (zmq::error_t& ex) {
        if (ex.num() != EINTR) {
          B2ERROR("There was an error during the proxy event: " << ex.what());
          running = false;
        }
      }
    }
    controlSocket.close();
    pubSocket.close();
    subSocket.close();
    context.close();
    B2DEBUG(10, "Proxy has finished");
    exit(0);
  }

  // Time to setup the proxy
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  m_streamer.initialize(0, true);
  m_client.initialize<ZMQ_PUB>(pubSocketAddress, subSocketAddress, controlSocketAddress, false);
  m_client.subscribe(c_MessageTypes::c_helloMessage);
  m_client.subscribe(c_MessageTypes::c_statisticMessage);
  m_client.subscribe(c_MessageTypes::c_deathMessage);
  m_client.subscribe(c_MessageTypes::c_terminateMessage);

  B2DEBUG(10, "Started multicast publishing on " << pubSocketAddress << " and subscribing on " << subSocketAddress);
}

void ProcessMonitor::terminate()
{
  m_client.terminate(false);
}

void ProcessMonitor::reset()
{
  m_client.reset();
}

void ProcessMonitor::killProcesses(unsigned int timeout)
{
  B2ASSERT("Only the monitoring process is allowed to kill processes", GlobalProcHandler::isProcess(ProcType::c_Monitor)
           or GlobalProcHandler::isProcess(ProcType::c_Init));

  if (not m_processList.empty() and m_client.isOnline()) {
    B2DEBUG(10, "Try to kill the processes gently...");
    // Try to kill them gently...
    auto pcbMulticastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_stopMessage);
    m_client.publish(std::move(pcbMulticastMessage));

    checkChildProcesses();

    const auto multicastAnswer = [this](const auto & socket) {
      processMulticast(socket);
      for (const auto& pair : m_processList) {
        if (pair.second != ProcType::c_Stopped) {
          B2DEBUG(10, "Process pid " << pair.first << " of type " << static_cast<char>(pair.second) << " is still alive");
          return true;
        }
      }
      return not m_receivedStatistics;
    };

    bool allProcessesStopped = true;
    for (const auto& pair : m_processList) {
      if (pair.second != ProcType::c_Stopped) {
        allProcessesStopped = false;
        break;
      }
    }

    if (not allProcessesStopped) {
      B2DEBUG(10, "Start waiting for processes to go down.");
      m_client.pollMulticast(timeout * 1000, multicastAnswer);
      B2DEBUG(10, "Finished waiting for processes to go down.");
    }
  }

  if (m_client.isOnline()) {
    B2DEBUG(10, "Will kill the proxy now.");
    // Kill the proxy and give it some time to terminate
    auto message = ZMQMessageHelper::createZMQMessage("TERMINATE");
    m_client.send(message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // If everything did not help, we will kill all of them
  GlobalProcHandler::killAllProcesses();
}

void ProcessMonitor::waitForRunningInput(const int timeout)
{
  if (processesWithType(ProcType::c_Input) < 1) {
    const auto multicastAnswer = [this](const auto & socket) {
      processMulticast(socket);
      return processesWithType(ProcType::c_Input) < 1;
    };

    const auto pullResult = m_client.pollMulticast(timeout * 1000, multicastAnswer);
    if (not pullResult) {
      B2ERROR("Input process did not start properly!");
      m_hasEnded = true;
    }
  }
}

void ProcessMonitor::waitForRunningWorker(const int timeout)
{
  if (processesWithType(ProcType::c_Worker) < m_requestedNumberOfWorkers) {
    const auto multicastAnswer = [this](const auto & socket) {
      processMulticast(socket);
      return processesWithType(ProcType::c_Worker) < m_requestedNumberOfWorkers;
    };

    const auto pullResult = m_client.pollMulticast(timeout * 1000, multicastAnswer);
    if (not pullResult) {
      B2ERROR("Worker process did not start properly!");
      m_hasEnded = true;
    }
  }
}

void ProcessMonitor::waitForRunningOutput(const int timeout)
{
  if (processesWithType(ProcType::c_Output) < 1) {
    const auto multicastAnswer = [this](const auto & socket) {
      processMulticast(socket);
      return processesWithType(ProcType::c_Output) < 1;
    };

    const auto pullResult = m_client.pollMulticast(timeout * 1000, multicastAnswer);
    if (not pullResult) {
      B2ERROR("Output process did not start properly!");
      m_hasEnded = true;
    }
  }
}

void ProcessMonitor::initialize(unsigned int requestedNumberOfWorkers)
{
  m_requestedNumberOfWorkers = requestedNumberOfWorkers;
}

void ProcessMonitor::checkMulticast(int timeout)
{
  if (hasEnded()) {
    return;
  }

  const auto multicastAnswer = [this](const auto & socket) {
    processMulticast(socket);
    return false;
  };
  m_client.pollMulticast(timeout * 1000, multicastAnswer);
}

template <class ASocket>
void ProcessMonitor::processMulticast(const ASocket& socket)
{
  auto pcbMulticastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
  if (pcbMulticastMessage->isMessage(c_MessageTypes::c_helloMessage)) {
    const int pid = std::stoi(pcbMulticastMessage->getData());
    const ProcType procType = GlobalProcHandler::getProcType(pid);
    m_processList[pid] = procType;
    B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Input) << " input processes.");
    B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Output) << " output processes.");
    B2DEBUG(10, "Now having " << processesWithType(ProcType::c_Worker) << " worker processes.");
  } else if (pcbMulticastMessage->isMessage(c_MessageTypes::c_terminateMessage)) {
    const int pid = std::stoi(pcbMulticastMessage->getData());
    const auto& processIt = m_processList.find(pid);
    if (processIt == m_processList.end()) {
      B2WARNING("An unknown PID died!");
      return;
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
  } else if (pcbMulticastMessage->isMessage(c_MessageTypes::c_statisticMessage)) {
    m_streamer.read(std::move(pcbMulticastMessage));
    B2DEBUG(10, "Having received the process statistics");
    m_receivedStatistics = true;
  }
}

void ProcessMonitor::checkChildProcesses()
{
  if (hasEnded()) {
    return;
  }

  // Copy is intended, as we do not want the signal handler to change our list
  std::vector<int> currentProcessList = GlobalProcHandler::getPIDList();
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
      B2ERROR("An input process has died unexpected! Need to go down.");
      m_hasEnded = true;
      return;
    } else if (pair.second == ProcType::c_Output) {
      B2ERROR("An output process has died unexpected! Need to go down.");
      m_hasEnded = true;
      return;
    } else if (pair.second == ProcType::c_Proxy) {
      B2ERROR("A proxy process has died unexpected! Need to go down.");
      m_hasEnded = true;
      return;
    } else if (pair.second == ProcType::c_Worker) {
      B2WARNING("A worker process has died unexpected. If you have requested, I will now restart the workers.");
      B2ASSERT("A worker died but none was present?", processesWithType(ProcType::c_Worker) != 0);
      auto pcbMulticastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_deleteMessage, pair.first);
      m_client.publish(std::move(pcbMulticastMessage));
    } else if (pair.second == ProcType::c_Stopped) {
      B2DEBUG(10, "An children process has died expectedly.");
    }

    iter = m_processList.erase(iter);
  }

  m_hasEnded = false;
  // The processing should be finished, if...
  if (m_processList.empty()) {
    // .. there is no process around anymore
    m_hasEnded = true;
  } else if (m_processList.size() == 1 and m_processList.begin()->second == ProcType::c_Output) {
    // ... the single remaining process is an output process (can happen, if we do not use the event backup)
    m_hasEnded = true;
  } else {
    // ... there are only workers or stopped processes around
    m_hasEnded = true;
    for (const auto& pair : m_processList) {
      if (pair.second == ProcType::c_Input or pair.second == ProcType::c_Output) {
        m_hasEnded = false;
        break;
      }
    }
  }

  if (m_hasEnded) {
    B2DEBUG(10, "No input and no output process around. Will go home now!");
  }
}

void ProcessMonitor::checkSignals(int g_signalReceived)
{
  if (g_signalReceived > 0) {
    B2DEBUG(10, "Received signal " << g_signalReceived);
    m_hasEnded = true;
  }
}

bool ProcessMonitor::hasEnded() const
{
  return m_hasEnded;
}

bool ProcessMonitor::hasWorkers() const
{
  return processesWithType(ProcType::c_Worker) > 0;
}

unsigned int ProcessMonitor::needMoreWorkers() const
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