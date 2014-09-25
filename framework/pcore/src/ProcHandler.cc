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


using namespace std;
using namespace Belle2;

/// Set static process ID number
int ProcHandler::s_processID = -1;

ProcHandler::ProcHandler() { }

ProcHandler::~ProcHandler() { }

void ProcHandler::startInputProcess()
{
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {   // Mother process
    m_inputProcessList.push_back(pid);
    B2INFO("ProcHandler: input process forked. pid = " << pid);
  } else if (pid < 0) {
    B2FATAL("fork() failed: " << strerror(errno));
  } else {
    s_processID = 10000;
    //die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
  }
}

void ProcHandler::startEventProcesses(int nproc)
{
  for (int i = 0; i < nproc; i++) {
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid > 0) {   // Mother process
      m_eventProcessList.push_back(pid);
      B2INFO("ProcHandler: event process " << i << " forked. pid = " << pid);
      fflush(stdout);
    } else if (pid < 0) {
      B2FATAL("fork() failed: " << strerror(errno));
    } else { // Event Process
      s_processID = i;
      //die when parent dies
      prctl(PR_SET_PDEATHSIG, SIGHUP);
      break;
    }
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

bool ProcHandler::isEventProcess()
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
  if (isEventProcess())
    return "event";
  if (isInputProcess())
    return "input";
  if (isOutputProcess())
    return "output";

  //shouldn't happen
  return "???";
}


void ProcHandler::waitForProcesses(std::vector<pid_t>& pids)
{
  for (pid_t pid : pids) {
    while (1) {
      int status;
      int ret = waitpid(pid, &status, 0);
      if (ret == -1) {
        if (errno == EINTR) {
          continue; //interrupted, try again
        } else if (errno == ECHILD) {
          break; //doesn't exist anymore, so nothing to do
        } else {
          B2FATAL("waitpid(" << pid << ") failed: " << strerror(errno));
        }
      } else {
        //success
        break;
      }
    }
  }
  pids.clear();
}

void ProcHandler::waitForAllProcesses()
{
  waitForInputProcesses();
  waitForEventProcesses();
  waitForOutputProcesses();
}

void ProcHandler::waitForInputProcesses() { waitForProcesses(m_inputProcessList); }
void ProcHandler::waitForEventProcesses() { waitForProcesses(m_eventProcessList); }
void ProcHandler::waitForOutputProcesses() { waitForProcesses(m_outputProcessList); }
