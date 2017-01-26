/// @file ProcHandler.cc
/// @brief Process handler class implementation
/// @author Soohyung Lee
/// @date Jul 14 2008

#include <framework/pcore/ProcHandler.h>
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <framework/core/EventProcessor.h>

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


using namespace std;
using namespace Belle2;

namespace {
  static int s_processID = -1;
  static int s_numEventProcesses = 0;

  // global list of PIDs managed by ProcHandler.
  // (directly modifying STL structures in the signal handler is unsafe, so let's be overly
  // cautious and use only C-like functions there.)
  // PIDs are addedusing addPID() while forking, items are set to 0 when process stops
  static std::vector<int> s_pidVector;
  static int* s_pids = nullptr;
  static int s_numpids = 0;
  void addPID(int pid)
  {
    s_pidVector.reserve(20); //TODO: only for testing wether this avoids what I think is a race condition. (happens only on buildbot)
    s_pidVector.push_back(pid);
    s_pids = s_pidVector.data();
    s_numpids = s_pidVector.size();
  }

  //in signal handler, use only the following functions, or s_numpids
  bool containsPID(int pid)
  {
    for (int i = 0; i < s_numpids; i++)
      if (s_pids[i] == pid)
        return true;
    return false;
  }
  void removePID(int pid)
  {
    for (int i = 0; i < s_numpids; i++)
      if (s_pids[i] == pid)
        s_pids[i] = 0;

    //set s_numpids if empty (or stop searching)
    for (int i = 0; i < s_numpids; i++)
      if (s_pids[i] != 0)
        return;
    s_numpids = 0;
  }
  void clearPIDs()
  {
    for (int i = 0; i < s_numpids; i++)
      s_pids[i] = 0;
    s_numpids = 0;
  }

  void sigChldHandler(int)
  {
    int raiseSig = 0;
    while (s_numpids != 0) {
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
      } else if (pid == 0) {
        //further children exist, but no state change yet
        break;
      } else { //state change
        //errors?
        if (WIFSIGNALED(status)) {
          //ok, it died because of some signal
          //EventProcessor::writeToStdErr("\nOne of our child processes died, stopping execution...\n");
          raiseSig = WTERMSIG(status);

          //backtrace in parent is not helpful
          if (raiseSig == SIGSEGV)
            raiseSig = SIGTERM;
        } else if (WIFEXITED(status) and WEXITSTATUS(status) != 0) {
          EventProcessor::writeToStdErr("\nExecution stopped, sub-process exited with non-zero exit status. Please check other log messages for details.\n");
          raiseSig = SIGTERM;
        }

        //remove pid from global list
        removePID(pid);
      }
    }

    if (raiseSig)
      raise(raiseSig);
  }


  /** Start a new process, adding its PID to processList, and setting s_processID = id. */
  bool startProc(std::set<int>* processList, const std::string& procType, int id)
  {
    static bool s_signalHandlerInstalled = false;
    if (not s_signalHandlerInstalled) {
      EventProcessor::installSignalHandler(SIGCHLD, sigChldHandler);
      s_signalHandlerInstalled = true;
    }

    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid > 0) {   // Mother process
      processList->insert(pid);
      addPID(pid);
      B2INFO("ProcHandler: " << procType << " process forked. pid = " << pid);
      fflush(stdout);
    } else if (pid < 0) {
      B2FATAL("fork() failed: " << strerror(errno));
    } else {
      //do NOT handle SIGCHLD in forked processes!
      EventProcessor::installSignalHandler(SIGCHLD, SIG_IGN);

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

}

ProcHandler::ProcHandler() { }
ProcHandler::~ProcHandler() { }

void ProcHandler::startInputProcess()
{
  startProc(&m_processList, "input", 10000);
}

void ProcHandler::startWorkerProcesses(int nproc)
{
  for (int i = 0; i < nproc; i++) {
    if (startProc(&m_processList, "worker", i))
      break; // in child process
  }
  s_numEventProcesses = nproc;
}

void ProcHandler::startOutputProcess() { s_processID = 20000; }

bool ProcHandler::parallelProcessingUsed() { return s_processID != -1; }

bool ProcHandler::isInputProcess() { return (s_processID >= 10000 and s_processID < 20000); }

bool ProcHandler::isWorkerProcess() { return (parallelProcessingUsed() and s_processID < 10000); }

bool ProcHandler::isOutputProcess() { return s_processID >= 20000; }

int ProcHandler::numEventProcesses()
{
  return s_numEventProcesses;
}

std::set<int> ProcHandler::globalProcessList()
{
  return std::set<int>(s_pidVector.begin(), s_pidVector.end());
}
std::set<int> ProcHandler::processList() const
{
  return m_processList;
}

int ProcHandler::EvtProcID() { return s_processID; }

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
  while (!m_processList.empty()) {
    for (int pid : m_processList) {
      //once a process is gone from the global list, remove them from our own, too.
      if (!containsPID(pid)) {
        m_processList.erase(pid);
        break;
      }
    }

    usleep(100);
  }
}
