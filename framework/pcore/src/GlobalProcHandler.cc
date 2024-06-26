/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/GlobalProcHandler.h>
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <framework/core/EventProcessor.h>

#include <framework/pcore/ProcHandler.h>

#include <sys/wait.h>
#include <sys/prctl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <Python.h>

#include <thread>
#include <chrono>

#include <iostream>

using namespace std;
using namespace Belle2;

ProcType GlobalProcHandler::s_procType = ProcType::c_Init;
int GlobalProcHandler::s_processID = -1;
int GlobalProcHandler::s_numEventProcesses = 0;

std::vector<int> GlobalProcHandler::s_pidVector;
std::map<int, ProcType> GlobalProcHandler::s_startedPIDs;

void GlobalProcHandler::childSignalHandler(int)
{
  while (!GlobalProcHandler::pidListEmpty()) {
    int status;
    int pid = waitpid(-1, &status, WNOHANG);
    if (pid == -1) {
      if (errno == EINTR) {
        continue; // interrupted, try again
      } else if (errno == ECHILD) {
        // We don't have any child processes?
        EventProcessor::writeToStdErr("\n Called waitpid() without any children left. This shouldn't happen and and indicates a problem.\n");
        // actually, this is ok in case we already called waitpid() somewhere else. (but we don't do that...)
        // In case I want to avoid this, waitid() and WNOWAIT might help, but require libc >= 2.12 (not present in SL5)
        GlobalProcHandler::clearPIDs();
        return;
      } else {
        // also shouldn't happen
        EventProcessor::writeToStdErr("\nwaitpid() failed.\n");
      }
    } else if (pid == 0) {
      // should not happen because of waitpid(-1,...)
      // further children exist, but no state change yet
      break;
    } else {
      // state change
      // get signed PID
      if (not GlobalProcHandler::findPID(pid)) {
        // unknown child process died, ignore
        continue;
      }

      // errors?
      if (WIFSIGNALED(status) or (WIFEXITED(status) and WEXITSTATUS(status) != 0)) {
        EventProcessor::writeToStdErr("\nSub-process exited with non-zero exit status. Please check other log messages for details.\n");
      }

      // remove pid from global list
      GlobalProcHandler::removePID(pid);
    }
  }
}


void GlobalProcHandler::addPID(int newPID)
{
  // if possible, insert pid into gap in list
  for (int& pid : s_pidVector) {
    if (pid == 0) {
      pid = newPID;
      return;
    }
  }

  B2FATAL("PID vector at capacity. This produces a race condition, make sure GlobalProcHandler is created early.");
}

bool GlobalProcHandler::findPID(int pid)
{
  return std::find(s_pidVector.begin(), s_pidVector.end(), pid) != s_pidVector.end();
}

void GlobalProcHandler::removePID(int oldPID)
{
  for (int& pid : s_pidVector) {
    if (pid == oldPID) {
      pid = 0;
      return;
    }
  }
}

void GlobalProcHandler::clearPIDs()
{
  std::fill(s_pidVector.begin(), s_pidVector.end(), 0);
}

bool GlobalProcHandler::pidListEmpty()
{
  for (const int& pid : s_pidVector) {
    if (pid != 0) {
      return false;
    }
  }
  return true;
}

void GlobalProcHandler::initialize(unsigned int nWorkerProc)
{
  B2ASSERT("Constructing GlobalProcHandler after forking is not allowed!", pidListEmpty());

  s_numEventProcesses = nWorkerProc;

  // s_pidVector size shouldn't be changed once processes are forked (race condition)
  s_pidVector.resize(s_pidVector.size() + nWorkerProc + 3, 0); // num worker + input + output + proxy
}

bool GlobalProcHandler::startProxyProcess()
{
  return startProc(ProcType::c_Proxy, 30000);
}

bool GlobalProcHandler::startInputProcess()
{
  return startProc(ProcType::c_Input, 10000);
}

bool GlobalProcHandler::startWorkerProcesses(unsigned int numProcesses)
{
  for (unsigned int i = 0; i < numProcesses; i++) {
    if (startProc(ProcType::c_Worker, 0)) {
      return true;
    }
  }
  return false;
}

bool GlobalProcHandler::startOutputProcess(bool local)
{
  if (local) {
    s_procType = ProcType::c_Output;
    return true;
  } else {
    return (startProc(ProcType::c_Output, 20000));
  }
}

bool GlobalProcHandler::startMonitoringProcess()
{
  s_procType = ProcType::c_Monitor;
  return true;
}

bool GlobalProcHandler::parallelProcessingUsed()
{
  return s_processID != -1;
}

bool GlobalProcHandler::isProcess(ProcType procType)
{
  return (procType == s_procType);
}

bool GlobalProcHandler::startProc(ProcType procType, int id)
{
  EventProcessor::installSignalHandler(SIGCHLD, childSignalHandler);

  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {
    // Mother process
    addPID(pid);
    s_startedPIDs[pid] = procType;
    fflush(stdout);
  } else if (pid < 0) {
    B2FATAL("fork() failed: " << strerror(errno));
  } else {
    // Child process
    // do NOT handle SIGCHLD in forked processes!
    EventProcessor::installSignalHandler(SIGCHLD, SIG_IGN);

    s_procType = procType;

    if (id == 0)
      s_processID = getpid();
    else
      s_processID = id;

    ProcHandler::setProcessID(s_processID);    // Interface to existing ProcHandler

    // Reset some python state: signals, threads, gil in the child
    PyOS_AfterFork_Child();
    // InputController becomes useless in child process
    InputController::resetForChildProcess();
    // die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    return true;
  }
  return false;
}

std::string GlobalProcHandler::getProcessName()
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

int GlobalProcHandler::EvtProcID()
{
  return s_processID;
}

void GlobalProcHandler::killAllProcesses()
{
  for (int& pid : s_pidVector) {
    if (pid != 0) {
      if (kill(pid, SIGKILL) >= 0) {
        B2DEBUG(100, "hard killed process " << pid);
      } else {
        B2DEBUG(100, "no process " << pid << " found, already gone?");
      }
      pid = 0;
    }
  }
}

const std::vector<int>& GlobalProcHandler::getPIDList()
{
  return s_pidVector;
}

ProcType GlobalProcHandler::getProcType(int pid)
{
  const auto procTypeIt = s_startedPIDs.find(pid);
  if (procTypeIt == s_startedPIDs.end()) {
    B2FATAL("Asking for a non-existing PID");
  }
  return procTypeIt->second;
}

int GlobalProcHandler::numEventProcesses()
{
  return s_numEventProcesses;
}

bool GlobalProcHandler::isOutputProcess()
{
  return isProcess(ProcType::c_Output);
}

bool GlobalProcHandler::isWorkerProcess()
{
  return isProcess(ProcType::c_Worker);
}

bool GlobalProcHandler::isInputProcess()
{
  return isProcess(ProcType::c_Input);
}

void GlobalProcHandler::waitForAllProcesses()
{
  while (true) {
    if (pidListEmpty()) {
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
