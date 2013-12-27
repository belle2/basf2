/// @file ProcHandler.cc
/// @brief Process handler class implementation
/// @author Soohyung Lee
/// @date Jul 14 2008

#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string.h>


using namespace std;
using namespace Belle2;

/// Set static process ID number
int ProcHandler::s_fEvtProcID = -1;

ProcHandler::ProcHandler()
  : m_nEvtSrv(0), m_nEvtProc(0), m_nOutputSrv(0),
    m_fEvtServer(0), m_fEvtProc(0), m_fOutputSrv(0)
{

  m_kSrcShm = ftok(".", 'S');
  m_kOutShm = ftok(".", 'O');
}

/// @brief Destructor of ProcHandler class

ProcHandler::~ProcHandler()
{
}

/// @brief Initialize an event server

int ProcHandler::init_EvtServer()
{
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {   // Mother process
    m_lEvtSrv.push_back(pid);
    m_fEvtServer = 0;
    m_nEvtSrv++;
    B2INFO("ProcHandler: event server forked. pid = " << pid);
  } else if (pid < 0) {
    B2ERROR("init_EvtServer");
    exit(-99);
  } else {
    m_fEvtServer = 1; // I'm event server
    s_fEvtProcID = 10000;
    //die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
  }
  return 0;
}

int ProcHandler::init_EvtProc(int nproc)
{
  m_nEvtProc = nproc;
  for (int i = 0; i < nproc; i++) {
    if (m_fEvtProc == 0) {
      fflush(stdout);
      fflush(stderr);
      pid_t pid = fork();
      if (pid > 0) {   // Mother process
        m_lEvtProc.push_back(pid);
        m_fEvtProc = 0;
        m_nEvtProc++;
        B2INFO("ProcHandler: event process " << i << " forked. pid = " << pid);
        fflush(stdout);
      } else if (pid < 0) {
        B2ERROR("init_EvtProc");
        exit(-99);
      } else { // Event Process
        m_fEvtProc = 1;    // I'm event process
        s_fEvtProcID = i;
        //die when parent dies
        prctl(PR_SET_PDEATHSIG, SIGHUP);
      }
    }
  }
  return 0;
}

int ProcHandler::init_OutServer(int id)
{
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid > 0) {   // Mother process
    m_lOutputSrv.push_back(pid);
    m_fOutputSrv = 0;
    m_nOutputSrv++;
    B2INFO("ProcHandler: output server forked. pid = " << pid);
  } else if (pid < 0) {
    B2ERROR("init_EvtServer");
    exit(-99);
  } else {
    m_fOutputSrv = 1; // I'm output server
    m_fOutputSrvID = id;
    s_fEvtProcID = 20000 + id;
    //die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);
  }
  return 0;
}


/// @brief Check whether this process is framework or not
/// @return 0 for non-framework
/// @return 1 for framework
int ProcHandler::isFramework()
{
  return !(m_fEvtServer || m_fEvtProc || m_fOutputSrv);
}

/// @brief Check whether this process is event server or not
/// @return 0 for non-event server
/// @return 1 for event server
int ProcHandler::isEvtServer()
{
  return m_fEvtServer;
}

/// @brief Check whether this process is event process or not
/// @return 0 for non-event process
/// @return 1 for event process
int ProcHandler::isEvtProc()
{
  return m_fEvtProc;
}

/// @brief Check whether this process is output server or not
/// @return 0 for non-output server
/// @return 1 for output server
int ProcHandler::isOutputSrv()
{
  return m_fOutputSrv;
}

int ProcHandler::EvtProcID()
{
  return s_fEvtProcID;
}

/// @brief Get the key of the source shared memory
/// @return Key value of the source shared memory
key_t ProcHandler::get_srcShmKey()
{
  return m_kSrcShm;
}

/// @brief Get the key of the output shared memory
/// @return Key value of the output shared memory
key_t ProcHandler::get_outShmKey()
{
  return m_kOutShm;
}

// @brief Wait for all the forked processes completed
int ProcHandler::wait_processes()
{
  // wait for any process exit
  //  printf ( "... wait processes called from %d\n", getpid() );
  //  printf ( "sizes = EVS %d; EVP %d; OPS %d\n",
  //     m_lEvtSrv.size(), m_lEvtProc.size(), m_lOutputSrv.size() );
  //
  // Output server can be common for multiple sessions..
  //  while ( m_lEvtSrv.size() > 0 || m_lEvtProc.size() > 0  ||
  //    m_lOutputSrv.size() > 0 ) {

  while (!m_lEvtSrv.empty() || !m_lEvtProc.empty()) {
    while (1) {
      int status;
      int pid = waitpid(-1, &status, 0);
      if (pid == -1) {
        if (errno == EINTR)
          continue;
        else {
          B2ERROR("wait_processes : waitpid");
          return -1;
        }
      } else {
        //  printf ( "wait_processes : completion of %d detected\n", pid );
        remove_pid(pid);
        //  printf ( "sizes = EVS %d; EVP %d; OPS %d\n",
        //     m_lEvtSrv.size(), m_lEvtProc.size(), m_lOutputSrv.size() );
        break;
      }
    }
  }
  return 0;
}

// @brief scan pid list and remove the pid if found

int ProcHandler::remove_pid(pid_t pid)
{
  // Search for event_server list
  for (std::vector<pid_t>::iterator it = m_lEvtSrv.begin(); it != m_lEvtSrv.end(); ++it) {
    if (pid == *it) {
      m_lEvtSrv.erase(it);
      B2INFO("ProcHandler : event server " << pid << " completed and removed");
      return 0;
    }
  }
  // Search for output_server list
  for (std::vector<pid_t>::iterator it = m_lOutputSrv.begin(); it != m_lOutputSrv.end(); ++it) {
    if (pid == *it) {
      m_lOutputSrv.erase(it);
      B2INFO("ProcHandler : output server " << pid << " completed and removed");
      return 0;
    }
  }
  // Search for event process list
  for (std::vector<pid_t>::iterator it = m_lEvtProc.begin(); it != m_lEvtProc.end(); ++it) {
    if (pid == *it) {
      m_lEvtProc.erase(it);
      B2INFO("ProcHandler : event process " << pid << " completed and removed");
      return 0;
    }
  }
  return -1;
}

int ProcHandler::wait_event_server()
{
  unsigned int id = 0;
  while (id  < m_lEvtSrv.size()) {
    while (1) {
      int status;
      int pid = waitpid(m_lEvtSrv[id], &status, 0);
      if (pid == -1) {
        if (errno == EINTR)
          continue;
        else {
          B2ERROR("wait_processes : waitpid");
          return -1;
        }
      } else {
        break;
      }
    }
    id++;
  }
  m_lEvtSrv.erase(m_lEvtSrv.begin(), m_lEvtSrv.end());
  return 0;
}

int ProcHandler::wait_event_processes()
{
  unsigned int id = 0;
  while (id  < m_lEvtProc.size()) {
    while (1) {
      int status;
      int pid = waitpid(m_lEvtProc[id], &status, 0);
      if (pid == -1) {
        if (errno == EINTR)
          continue;
        else {
          B2ERROR("wait_processes : waitpid");
          return -1;
        }
      } else {
        break;
      }
    }
    id++;
  }
  m_lEvtProc.erase(m_lEvtProc.begin(), m_lEvtProc.end());
  return 0;
}

int ProcHandler::wait_output_server()
{
  unsigned int id = 0;
  while (id < m_lOutputSrv.size()) {
    while (1) {
      int status;
      int pid = waitpid(m_lOutputSrv[id], &status, 0);
      if (pid == -1) {
        if (errno == EINTR)
          continue;
        else {
          B2ERROR("wait_processes : waitpid");
          return -1;
        }
      } else {
        //  printf ( "wait_processes : completion of %d detected\n", pid );
        //        remove_pid(pid);
        //  printf ( "sizes = EVS %d; EVP %d; OPS %d\n",
        //     m_lEvtSrv.size(), m_lEvtProc.size(), m_lOutputSrv.size() );
        break;
      }
    }
    id++;
  }
  m_lOutputSrv.erase(m_lOutputSrv.begin(), m_lOutputSrv.end());
  return 0;
}
