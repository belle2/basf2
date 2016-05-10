/// @file ProcHandler.cc
/// @brief Process handler class implementation
/// @author Soohyung Lee
/// @date Jul 14 2008

#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <Python.h>


using namespace std;
using namespace Belle2;

/// Set static process ID number
int ProcHandler::s_processID = -1;

ProcHandler::ProcHandler() { }

ProcHandler::~ProcHandler() { }

bool ProcHandler::startProc(std::vector<pid_t>* processList, const std::string& procType, int id)
{
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {   // Mother process
    processList->push_back(pid);
    B2INFO("ProcHandler: " << procType << " process forked. pid = " << pid);
    fflush(stdout);
  } else if (pid < 0) {
    B2FATAL("fork() failed: " << strerror(errno));
  } else {
    ProcHandler::s_processID = id;
    //Reset some python state: signals, threads, gil in the child
    PyOS_AfterFork();
    //die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    return true;
  }
  return false;
}

void ProcHandler::startInputProcess()
{
  startProc(&m_inputProcessList, "input", 10000);
}

void ProcHandler::startWorkerProcesses(int nproc)
{
  for (int i = 0; i < nproc; i++) {
    if (startProc(&m_workerProcessList, "worker", i))
      break; // in child process
  }
}

void ProcHandler::startOutputProcess()
{
  s_processID = 20000;
}


bool ProcHandler::parallelProcessingUsed()
{
  return s_processID != -1;
}

bool ProcHandler::isInputProcess()
{
  return (s_processID >= 10000 and s_processID < 20000);
}

bool ProcHandler::isWorkerProcess()
{
  return (parallelProcessingUsed() and s_processID < 10000);
}

bool ProcHandler::isOutputProcess()
{
  return s_processID >= 20000;
}

int ProcHandler::EvtProcID()
{
  return s_processID;
}

std::string ProcHandler::getProcessName()
{
  if (isWorkerProcess())
    return "worker";
  if (isInputProcess())
    return "input";
  if (isOutputProcess())
    return "output";

  //shouldn't happen
  return "???";
}


void ProcHandler::waitForAllProcesses()
{
  unsigned nStopped = 0;
  unsigned nProcs = m_inputProcessList.size() + m_workerProcessList.size();
  while (nStopped < nProcs) {
    int status;
    int pid = waitpid(-1, &status, 0);
    if (pid == -1) {
      if (errno == EINTR) {
        continue; //interrupted, try again
      } else if (errno == ECHILD) {
        //actually this never happens?
        break; //no children exist, so nothing to do
      } else {
        B2FATAL("waitpid(" << pid << ") failed: " << strerror(errno));
      }
    } else { //state change
      if (WIFSIGNALED(status)) {
        B2FATAL("Execution stopped, sub-process with PID " << pid << " was killed by signal " << WTERMSIG(status) << ".");
      } else if (WIFEXITED(status) and WEXITSTATUS(status) != 0) {
        B2FATAL("Execution stopped, sub-process with PID " << pid << " has exit status " << WEXITSTATUS(status) << ".");
      } else {
        //process exited normally
        nStopped++;
      }
    }
  }
  m_inputProcessList.clear();
  m_workerProcessList.clear();
}
