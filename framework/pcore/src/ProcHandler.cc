/// @file ProcHandler.cc
/// @brief Process handler class implementation
/// @author Soohyung Lee
/// @date Jul 14 2008

#include <framework/pcore/ProcHelper.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <framework/core/EventProcessor.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ProcessedEventsBackupList.h>
#include <framework/pcore/zmq/proxy/ZMQMulticastProxy.h>

#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <Python.h>

#include <iostream>


using namespace std;
using namespace Belle2;

namespace {

  // input process procID: [10000-20000)
  // worker process procID: <10000
  // output crocess procID: >=20000
  static ProcType s_procType = ProcType::c_Init;
  static int s_processID = -1;
  static int s_numEventProcesses = 0;
  static int s_localChildrenWithErrors = 0;

  // global list of PIDs managed by ProcHandler.
  // (directly modifying STL structures in the signal handler is unsafe, so let's be overly
  // cautious and use only C-like functions there.)
  // PIDs are addedusing addPID() while forking, items are set to 0 when process stops
  static std::vector<int> s_pidVector;
  static int* s_pids = nullptr;
  static int s_numpids = 0;
  static int s_gKilledProc = 0;

  static ProcHandler* s_gProcHandler = nullptr;

  static std::unique_ptr<ZMQMulticastProxy> s_gPCBProxy;


  void shutdownPCBProxy(int signal)
  {
    if (s_gPCBProxy and signal == SIGUSR1) {
      //s_gPCBProxy->shutdown();
      //s_gPCBProxy.reset();
      exit(0);
    }
  }


  void addPID(int pid)
  {
    //if possible, insert pid into gap in list
    bool found_gap = false;
    for (int i = 0; i < s_numpids; i++) {
      if (s_pids[i] == 0) {
        found_gap = true;
        s_pids[i] = pid;
        break;
      }
    }

    if (!found_gap) {
      if (s_pidVector.size() == s_pidVector.capacity()) {
        B2FATAL("PID vector at capacity. This produces a race condition, make sure ProcHandler is created early.");
      }
      s_pidVector.push_back(pid);
    }
    s_pids = s_pidVector.data();
    s_numpids = s_pidVector.size();
  }

  //in signal handler, use only the following functions!
  /** returns 0 if not found, otherwise PID (or -PID for 'local' procs) */
  int findPID(int pid)
  {
    for (int i = 0; i < s_numpids; i++)
      if (std::abs(s_pids[i]) == std::abs(pid))
        return s_pids[i];
    return 0;
  }
  void removePID(int pid)
  {
    //B2DEBUG(100,"remove pid: " << pid);
    for (int i = 0; i < s_numpids; i++)
      if (std::abs(s_pids[i]) == std::abs(pid)) {
        s_pids[i] = 0;
      }
  }
  void clearPIDs()
  {
    for (int i = 0; i < s_numpids; i++)
      s_pids[i] = 0;
  }
  bool pidListEmpty()
  {
    for (int i = 0; i < s_numpids; i++)
      if (s_pids[i] != 0)
        return false;
    return true;
  }

  void sigChldHandler(int)
  {
    //EventProcessor::writeToStdErr("\n sigchild handler called .\n");
    int raiseSig = 0;
    while (!pidListEmpty()) {
      int status;
      int pid = waitpid(-1, &status, WNOHANG);
      if (pid == -1) {
        if (errno == EINTR) {
          continue; //interrupted, try again
        } else if (errno == ECHILD) {
          //We don't have any child processes?
          EventProcessor::writeToStdErr("\n Called waitpid() without any children left. This shouldn't happen and and indicates a problem.\n");
          //
          //actually, this is ok in case we already called waitpid() somewhere else. (but we don't do that...)
          //In case I want to avoid this, waitid() and WNOWAIT might help, but require libc >= 2.12 (not present in SL5)
          clearPIDs();
          return;
        } else {
          //also shouldn't happen
          EventProcessor::writeToStdErr("\nwaitpid() failed.\n");
        }
      } else if (pid == 0) { // should not happen because of waitpid(-1,...)
        //further children exist, but no state change yet
        break;
      } else { //state change
        //get signed PID
        pid = findPID(pid);
        if (pid == 0)
          continue; //unknown child process died, ignore


        int termSig = 0;
        //errors?
        if (WIFSIGNALED(status)) {
          //ok, it died because of some signal
          //EventProcessor::writeToStdErr("\nOne of our child processes died, stopping execution...\n");
          termSig = WTERMSIG(status);

          //backtrace in parent is not helpful
          if (termSig == SIGSEGV)
            termSig = SIGTERM;
        } else if (WIFEXITED(status) and WEXITSTATUS(status) != 0) {
          EventProcessor::writeToStdErr("\nExecution stopped, sub-process exited with non-zero exit status. Please check other log messages for details.\n");
          termSig = SIGTERM;
        }

        if (termSig != 0) {
          if (pid < 0)
            s_localChildrenWithErrors++;
          else
            raiseSig = termSig;
        }

        //remove pid from global list
        removePID(pid);
        if ((s_gKilledProc == 0 || s_gKilledProc != pid) && status != 0) {
          s_gProcHandler->sendPCBMessage(c_MessageTypes::c_deleteMessage, std::to_string(pid));
          s_gProcHandler->workerDied();
          B2ERROR("Child process (" << pid << ") finished unexpected with status " << status);
        } else { // we expected these SIGCHLD
          B2DEBUG(100, "Child process (" << pid << ") finished  expected with status " << status);
          s_gKilledProc = 0;
          if (status != 0) {
            s_gProcHandler->workerDied();
          }
        }
      }
    }

    //if (raiseSig)
    //  raise(raiseSig);
  }
}


// =====================================
// Constructor
// =====================================

ProcHandler::ProcHandler(unsigned int nWorkerProc, bool markChildrenAsLocal):
  m_markChildrenAsLocal(markChildrenAsLocal),
  m_numWorkerProcesses(nWorkerProc)
{
  if ((int)nWorkerProc > s_numEventProcesses)
    s_numEventProcesses = nWorkerProc;

  if (!pidListEmpty())
    B2FATAL("Constructing ProcHandler after forking is not allowed!");

  //s_pidVector size shouldn't be changed once processes are forked (race condition)
  s_pidVector.reserve(s_pidVector.size() + nWorkerProc + 3); // num worker + input + output + proxy
  s_pids = s_pidVector.data();

  if (isProcess(ProcType::c_Init)) {
    s_gProcHandler = this;
  }
}
ProcHandler::~ProcHandler() { }



// =================================
// Starting processes
// ==================================

void ProcHandler::startProxyProcess(const std::string& xpubProxySocketName, const std::string& xsubProxySocketName)
{
  if (s_procType == ProcType::c_Init) {
    if (startProc(&m_processList, "proxy", 30000)) {
      s_procType = ProcType::c_Proxy;
      //signal to shutdown the blocking proxy process
      EventProcessor::installSignalHandler(SIGUSR1, shutdownPCBProxy);

      s_gPCBProxy = std::make_unique<ZMQMulticastProxy>(xpubProxySocketName, xsubProxySocketName);
      B2DEBUG(100, "Start proxy work -> blocking");
      s_gPCBProxy->start();
    } else {
      sleep(2); // Time to setup the proxy
    }
  } else {
    B2FATAL("PCB Proxy Process not forked from basf2 Init Process");
  }
}


void ProcHandler::startInputProcess()
{
  if (startProc(&m_processList, "input", 10000)) {
    s_procType = ProcType::c_Input;
    m_context.release();
    m_pubSocket.release();
    m_subSocket.release();
    //stopPCBMulticast(); // Multicast for the Input Process will be setup in the ZMQ Modules
  }
}


void ProcHandler::startWorkerProcesses()
{
  for (unsigned int i = 0; i < m_numWorkerProcesses; i++) {
    if (startProc(&m_processList, "worker", 0)) {
      s_procType = ProcType::c_Worker;
      m_context.release();
      m_pubSocket.release();
      m_subSocket.release();
      //stopPCBMulticast(); // Multicast for the Worker Process will be setup in the ZMQ Modules
      break; // stop the forking loop in child process

    }
  }

}


void ProcHandler::startOutputProcess()
{
  if (s_processID == -1) {
    if (startProc(&m_processList, "output", 20000)) {
      s_procType = ProcType::c_Output;
      m_context.release();
      m_pubSocket.release();
      m_subSocket.release();
      //stopPCBMulticast(); // Multicast for the Output Process will be setup in the ZMQ Modules
    }
  }
}


void ProcHandler::restartWorkerProcess()
{
  B2RESULT("num restart worker: " << m_numRestartWorkers);
  while (m_numRestartWorkers > 0) {
    if (startProc(&m_processList, "worker", 0)) {
      s_procType = ProcType::c_Worker;
      m_context.release();
      m_pubSocket.release();
      m_subSocket.release();
      //stopPCBMulticast(); // Multicast for the Worker Process will be setup in the ZMQ Modules
      break; // stop the forking loop in child process

    } else {
      m_numRestartWorkers--;
    }
  }
}


bool ProcHandler::startProc(std::set<int>* processList, const std::string& procType, int id)
{
  EventProcessor::installSignalHandler(SIGCHLD, sigChldHandler);

  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {   // Mother process
    //if (m_markChildrenAsLocal)
    //  pid = -pid;
    processList->insert(pid);
    addPID(pid);
    B2INFO("ProcHandler: " << procType << " process forked. pid = " << pid);
    fflush(stdout);
  } else if (pid < 0) {
    B2FATAL("fork() failed: " << strerror(errno));
  } else {  // Child process
    //do NOT handle SIGCHLD in forked processes!
    EventProcessor::installSignalHandler(SIGCHLD, SIG_IGN);
    EventProcessor::installMainSignalHandlers(SIG_IGN);

    if (id == 0)
      s_processID = getpid();
    else
      s_processID = id;
    //Reset some python state: signals, threads, gil in the child
    PyOS_AfterFork();
    //InputController becomes useless in child process
    InputController::resetForChildProcess();
    //die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    return true;
  }
  return false;
}


void ProcHandler::setAsMonitoringProcess()
{
  s_procType = ProcType::c_Monitor;

}



// =================================
// PCB Multicast
// =================================

void ProcHandler::initPCBMulticast(const std::string& xpubProxySocketAddr, const std::string& xsubProxySocketAddr)
{
  m_context = std::make_unique<zmq::context_t>(1);
  m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
  m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);

  m_pubSocket->connect(xsubProxySocketAddr);
  m_subSocket->connect(xpubProxySocketAddr);

  m_pubSocket->setsockopt(ZMQ_LINGER, 0);
  m_subSocket->setsockopt(ZMQ_LINGER, 0);


  m_statusPCBMulticast = true;
}


void ProcHandler::stopPCBMulticast()
{
  if (isProcess(ProcType::c_Init) or isProcess(ProcType::c_Monitor)) {
    B2DEBUG(100, "Multicast stop...");
    if (m_subSocket) {
      m_subSocket->close();
      m_subSocket.reset();
    }
    if (m_pubSocket) {
      m_pubSocket->close();
      m_pubSocket.reset();
    }
    if (m_context) {
      m_context->close();
      m_context.reset();
    }
  } else {
    m_subSocket = nullptr;
    m_pubSocket = nullptr;
    m_context = nullptr;
  }
}


void ProcHandler::subscribePCBMulticast(c_MessageTypes filter)
{
  const char char_filter = static_cast<char>(filter);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &char_filter, 1);
}


bool ProcHandler::isPCBMulticast() { return m_statusPCBMulticast; }


void ProcHandler::sendPCBMessage(const Belle2::c_MessageTypes msgType, const std::string& data)
{
  if (isPCBMulticast()) {
    const auto& deleteMessage = ZMQMessageFactory::createMessage(msgType, data);
    deleteMessage->toSocket(m_pubSocket);
  } else {
    B2ERROR("ProcHandler tries to send PCB message while multicast is not set up yet");
  }
}


bool ProcHandler::parallelProcessingUsed() { return s_processID != -1; }


bool ProcHandler::isProcess(ProcType procType) { return (procType == s_procType);}


int ProcHandler::numEventProcesses() { return s_numEventProcesses; }


std::set<int> ProcHandler::globalProcessList() { return std::set<int>(s_pidVector.begin(), s_pidVector.end()); }
std::set<int> ProcHandler::processList() const { return m_processList; }


int ProcHandler::EvtProcID() { return s_processID; }


std::string ProcHandler::getProcessName()
{
  if (isProcess(ProcType::c_Worker))
    return "worker";
  if (isProcess(ProcType::c_Input))
    return "input";
  if (isProcess(ProcType::c_Output))
    return "output";
  if (isProcess(ProcType::c_Init))
    return "init";
  if (isProcess(ProcType::c_Monitor))
    return "monitor";

  //shouldn't happen
  return "???";
}

bool ProcHandler::checkProcessStatus()
{
  return m_processList.size() > 1 && isProcess(ProcType::c_Monitor);
}


bool ProcHandler::proceedPCBMulticast()
{
  if (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& pcbMulticastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (pcbMulticastMessage->isMessage(c_MessageTypes::c_deathMessage)) {
      B2DEBUG(100, "Got worker death message");
      int workerPID = atoi(pcbMulticastMessage->getData().c_str());
      if (kill(workerPID, SIGKILL) == 0) {
        B2WARNING("killed process " << workerPID);
      } else {
        B2ERROR("Try to kill process " << workerPID << " but process already gone");
      }
      // TODO: vector for security
      s_gKilledProc = workerPID;
    } else if (pcbMulticastMessage->isMessage(c_MessageTypes::c_terminateMessage)) {
      m_gotTerminateMsg = true;
      B2DEBUG(100, "Got terminate message");
    }
  }

  for (int pid : m_processList) {
    //once a process is gone from the global list, remove them from our own, too.
    if (findPID(pid) == 0) {
      m_processList.erase(pid);
      B2DEBUG(100, "deleted pid: " << pid);
      if (m_markChildrenAsLocal and pid < 0 and s_localChildrenWithErrors != 0) {
        //ok = false;
        s_localChildrenWithErrors--;
      }
      break;
    }
  }

  usleep(100);
  return m_numRestartWorkers > 0 && not m_gotTerminateMsg;
}


void ProcHandler::workerDied()
{
  m_numRestartWorkers++;
}

bool ProcHandler::waitForAllProcesses()
{
  bool ok = true;
  while (m_processList.size() > 1 && isProcess(ProcType::c_Monitor)) { // the last process in the pid list is the proxy process
    if (ZMQHelper::pollSocket(m_subSocket, 0)) {
      const auto& pcbMulticastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
      if (pcbMulticastMessage->isMessage(c_MessageTypes::c_deathMessage)) {
        B2DEBUG(100, "Got worker death message");
        int workerPID = atoi(pcbMulticastMessage->getData().c_str());
        if (kill(workerPID, SIGKILL) == 0) {
          B2WARNING("killed process " << workerPID);
        } else {
          B2ERROR("Try to kill process " << workerPID << " but process already gone");
        }
        // TODO: vector for security
        s_gKilledProc = workerPID;
      }
    }

    for (int pid : m_processList) {
      //once a process is gone from the global list, remove them from our own, too.
      if (findPID(pid) == 0) {
        m_processList.erase(pid);
        B2DEBUG(100, "deleted pid: " << pid);
        if (m_markChildrenAsLocal and pid < 0 and s_localChildrenWithErrors != 0) {
          ok = false;
          s_localChildrenWithErrors--;
        }
        break;
      }
    }

    usleep(100);
  }
  return ok;
}


bool ProcHandler::waitForStartEvtProc()
{
  bool inputOnline = false;
  bool outputOnline = false;

  while (not inputOnline or not outputOnline) {
    if (ZMQHelper::pollSocket(m_subSocket, 10000)) {
      const auto& pcbMulticastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
      if (pcbMulticastMessage->isMessage(c_MessageTypes::c_helloMessage)) {
        if (pcbMulticastMessage->getData() == "input") {
          B2DEBUG(100, "Monitoring: received hello from input");
          inputOnline = true;
          continue;
        } else if (pcbMulticastMessage->getData() == "output") {
          B2DEBUG(100, "Monitoring: received hello from output");
          outputOnline = true;
          continue;
        } else {
          B2ERROR("unexpected hello message on multicast");
          return false;
        }
      }
    } else {
      B2ERROR("Timeout while waiting for input and output processes");
      return false;
    }
  }
  //ZMQHelper::socketSniffer(m_subSocket, 1000, 20);
  return true; // turn to ture when uncomment
}


void ProcHandler::startEvtProc()
{
  std::string strNumWorker(std::to_string(m_numWorkerProcesses));
  const auto& pcbMulticastStartMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_startMessage, strNumWorker);
  pcbMulticastStartMsg->toSocket(m_pubSocket);
}


void ProcHandler::stopEvtProc()
{
  const auto& pcbMulticastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_terminateMessage);
}


void ProcHandler::killAllChildProc(unsigned int timeout)
{
  if ((isProcess(ProcType::c_Monitor) or isProcess(ProcType::c_Init)) and sizeof(s_pids) > 0) {
    if (s_pids[0] > 0) {
      B2DEBUG(100, "Interrupt (SIGUSR1) proxy: " << s_pids[0]);
      kill(s_pids[0], SIGUSR1); // interrupt and shutdown the blocking proxy
    }
    sleep(timeout);

    // hard kill of all processes left in process list
    for (int i = 0; i < s_numpids; i++) {
      if (s_pids[i] != 0) {
        if (kill(s_pids[i], SIGKILL) >= 0) {
          B2DEBUG(100, "hard killed process " << s_pids[i]);
        } else {
          B2DEBUG(100, "no process " << s_pids[i] << " found, already gone?");
        }
        s_pids[i] = 0;
      }
    }


  } else { return;}
}



bool ProcHandler::isOutputProcess()
{
  return ProcHandler::isProcess(ProcType::c_Output);
}

bool ProcHandler::isWorkerProcess()
{
  return ProcHandler::isProcess(ProcType::c_Worker);
}




