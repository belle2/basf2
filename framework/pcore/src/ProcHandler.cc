/// @file ProcHandler.cc
/// @brief Process handler class implementation
/// @author Soohyung Lee
/// @date Jul 14 2008

#include <framework/pcore/ProcHandler.h>
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>
#include <framework/core/EventProcessor.h>

#include <set>

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
  /// Set static process ID number
  static int s_processID = -1;
  // Set static number of processes
  static int s_nproc = 0;
  // global list of PIDs managed by ProcHandler.
  static std::set<int> s_pidList;

  static bool s_signalHandlerInstalled = false;

  void sigChldHandler(int)
  {
    int raiseSig = 0;
    while (!s_pidList.empty()) {
      int status;
      int pid = waitpid(-1, &status, WNOHANG);
      if (pid == -1) {
        if (errno == EINTR) {
          continue; //interrupted, try again
        } else if (errno == ECHILD) {
          //We don't have any child processes?
          //EventProcessor::writeToStdErr("\n Called waitpid() without any children left. This shouldn't happen and and indicates a problem.\n");
          //
          //actually, this is ok in case we already called waitpid() somewhere else.
          //In case I want to avoid this, waitid() and WNOWAIT might help, but require libc >= 2.12 (not present in SL5)
          s_pidList.clear();
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
        s_pidList.erase(pid);
      }
    }

    if (raiseSig)
      raise(raiseSig);

    //TODO do what exactly?
    //a)
    //cleanup && raise()
    //TODO what actually is cleanup here? how does this prevent deadlocks?
    //b) kill all children? and wait?
    //

  }


  /** Start a new process, adding its PID to processList, and setting s_processID = id. */
  bool startProc(std::set<int>* processList, const std::string& procType, int id)
  {
    if (not s_signalHandlerInstalled) {
      EventProcessor::installSignalHandler(SIGCHLD, sigChldHandler);
      s_signalHandlerInstalled = true;
    }

    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid > 0) {   // Mother process
      processList->insert(pid);
      s_pidList.insert(pid);
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
  s_nproc = nproc;
}

void ProcHandler::startOutputProcess() { s_processID = 20000; }

bool ProcHandler::parallelProcessingUsed() { return s_processID != -1; }

bool ProcHandler::isInputProcess() { return (s_processID >= 10000 and s_processID < 20000); }

bool ProcHandler::isWorkerProcess() { return (parallelProcessingUsed() and s_processID < 10000); }

bool ProcHandler::isOutputProcess() { return s_processID >= 20000; }

int ProcHandler::numEventProcesses()
{
  return s_nproc;
}

std::set<int> ProcHandler::globalProcessList()
{
  return s_pidList;
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
      if (s_pidList.count(pid) == 0) {
        m_processList.erase(pid);
        break;
      }
    }

    usleep(100);
  }
}
