/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee, Anseln Baur, Nils Braun                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/ProcHandler.h>
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <framework/core/EventProcessor.h>

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

ProcType ProcHandler::s_procType = ProcType::c_Init;
int ProcHandler::s_processID = -1;
int ProcHandler::s_numEventProcesses = 0;

std::vector<int> ProcHandler::s_pidVector;
std::map<int, ProcType> ProcHandler::s_startedPIDs;

void ProcHandler::childSignalHandler(int)
{
  while (!ProcHandler::pidListEmpty()) {
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
        ProcHandler::clearPIDs();
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
      if (not ProcHandler::findPID(pid)) {
        // unknown child process died, ignore
        continue;
      }

      // errors?
      if (WIFSIGNALED(status) or (WIFEXITED(status) and WEXITSTATUS(status) != 0)) {
        EventProcessor::writeToStdErr("\nSub-process exited with non-zero exit status. Please check other log messages for details.\n");
      }

      // remove pid from global list
      ProcHandler::removePID(pid);
    }
  }
}


void ProcHandler::addPID(int newPID)
{
  // if possible, insert pid into gap in list
  for (int& pid : s_pidVector) {
    if (pid == 0) {
      pid = newPID;
      return;
    }
  }

  B2FATAL("PID vector at capacity. This produces a race condition, make sure ProcHandler is created early.");
}

bool ProcHandler::findPID(int pid)
{
  return std::find(s_pidVector.begin(), s_pidVector.end(), pid) != s_pidVector.end();
}

void ProcHandler::removePID(int oldPID)
{
  for (int& pid : s_pidVector) {
    if (pid == oldPID) {
      pid = 0;
      return;
    }
  }
}

void ProcHandler::clearPIDs()
{
  std::fill(s_pidVector.begin(), s_pidVector.end(), 0);
}

bool ProcHandler::pidListEmpty()
{
  for (const int& pid : s_pidVector) {
    if (pid != 0) {
      return false;
    }
  }
  return true;
}

void ProcHandler::initialize(unsigned int nWorkerProc)
{
  B2ASSERT("Constructing ProcHandler after forking is not allowed!", pidListEmpty());

  s_numEventProcesses = nWorkerProc;

  // s_pidVector size shouldn't be changed once processes are forked (race condition)
  s_pidVector.resize(s_pidVector.size() + nWorkerProc + 3, 0); // num worker + input + output + proxy
}

bool ProcHandler::startProxyProcess()
{
  return startProc(ProcType::c_Proxy, 30000);
}

bool ProcHandler::startInputProcess()
{
  return startProc(ProcType::c_Input, 10000);
}

bool ProcHandler::startWorkerProcesses(unsigned int numProcesses)
{
  for (int i = 0; i < numProcesses; i++) {
    if (startProc(ProcType::c_Worker, 0)) {
      return true;
    }
  }
  return false;
}

bool ProcHandler::startOutputProcess()
{
  return (startProc(ProcType::c_Output, 20000));
}

bool ProcHandler::startMonitoringProcess()
{
  s_procType = ProcType::c_Monitor;
  return true;
}

bool ProcHandler::parallelProcessingUsed()
{
  return s_processID != -1;
}

bool ProcHandler::isProcess(ProcType procType)
{
  return (procType == s_procType);
}

bool ProcHandler::startProc(ProcType procType, int id)
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
    // TODO: Also ignore all other main signals?
    EventProcessor::installMainSignalHandlers(SIG_IGN);

    s_procType = procType;

    if (id == 0)
      s_processID = getpid();
    else
      s_processID = id;

    // Reset some python state: signals, threads, gil in the child
    PyOS_AfterFork();
    // InputController becomes useless in child process
    InputController::resetForChildProcess();
    // die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    return true;
  }
  return false;
}

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

int ProcHandler::EvtProcID()
{
  return s_processID;
}

void ProcHandler::killAllProcesses()
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

const std::vector<int>& ProcHandler::getPIDList()
{
  return s_pidVector;
}

ProcType ProcHandler::getProcType(int pid)
{
  const auto procTypeIt = s_startedPIDs.find(pid);
  if (procTypeIt == s_startedPIDs.end()) {
    B2FATAL("Asking for a non-existing PID");
  }
  return procTypeIt->second;
}

unsigned int ProcHandler::numEventProcesses()
{
  return s_numEventProcesses;
}

bool ProcHandler::isOutputProcess()
{
  return isProcess(ProcType::c_Output);
}

bool ProcHandler::isWorkerProcess()
{
  return isProcess(ProcType::c_Worker);
}