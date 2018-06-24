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

namespace {
  static ProcType s_procType = ProcType::c_Init;
  static int s_processID = -1;
  static int s_numEventProcesses = 0;

  // global list of PIDs managed by ProcHandler.
  // (directly modifying STL structures in the signal handler is unsafe, so let's be overly
  // cautious and use only C-like functions there.)
  // PIDs are added using addPID() while forking, items are set to 0 when process stops
  static std::vector<int> s_pidVector;
  static int* s_pids = nullptr;
  static int s_numpids = 0;

  static ProcHandler* s_gProcHandler = nullptr;

  void addPID(int pid)
  {
    // if possible, insert pid into gap in list
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

// returns 0 if not found, otherwise PID
  int findPID(int pid)
  {
    for (int i = 0; i < s_numpids; i++) {
      if (std::abs(s_pids[i]) == std::abs(pid)) {
        return s_pids[i];
      }
    }
    return 0;
  }

  void removePID(int pid)
  {
    for (int i = 0; i < s_numpids; i++) {
      if (std::abs(s_pids[i]) == std::abs(pid)) {
        s_pids[i] = 0;
      }
    }
  }

  void clearPIDs()
  {
    for (int i = 0; i < s_numpids; i++) {
      s_pids[i] = 0;
    }
  }

  bool pidListEmpty()
  {
    for (int i = 0; i < s_numpids; i++) {
      if (s_pids[i] != 0) {
        return false;
      }
    }
    return true;
  }

// This signal handler is called when a child process dies/is changed
  void childSignalHandler(int)
  {
    while (!pidListEmpty()) {
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
          clearPIDs();
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
        pid = findPID(pid);
        if (pid == 0) {
          // unknown child process died, ignore
          continue;
        }

        // errors?
        if (WIFSIGNALED(status) or (WIFEXITED(status) and WEXITSTATUS(status) != 0)) {
          EventProcessor::writeToStdErr("\nExecution stopped, sub-process exited with non-zero exit status. Please check other log messages for details.\n");
        }

        // remove pid from global list
        removePID(pid);
      }
    }
  }
}

ProcHandler::ProcHandler(unsigned int nWorkerProc) : m_numWorkerProcesses(nWorkerProc)
{
  if ((int)nWorkerProc > s_numEventProcesses)
    s_numEventProcesses = nWorkerProc;

  if (!pidListEmpty())
    B2FATAL("Constructing ProcHandler after forking is not allowed!");

  // s_pidVector size shouldn't be changed once processes are forked (race condition)
  s_pidVector.reserve(s_pidVector.size() + nWorkerProc + 3); // num worker + input + output + proxy
  s_pids = s_pidVector.data();

  if (isProcess(ProcType::c_Init)) {
    s_gProcHandler = this;
  }
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
}

const std::vector<int>& ProcHandler::getPIDList() const
{
  return s_pidVector;
}