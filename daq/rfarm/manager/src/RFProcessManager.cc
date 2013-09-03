//+
// File : RFProcessManager.cc
// Description : Base class for RFARM node management
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 20 - June - 2013
//-

#include "daq/rfarm/manager/RFProcessManager.h"
#include <time.h>

using namespace std;
using namespace Belle2;

// Constructor/Destructor

// *** OLD ***
//RFProcessManager::RFProcessManager( string& nodename, string& conffile, string& infofile ) :
//  RFSharedMem (nodename.c_str()), RFNSM (nodename.c_str(), infofile.c_str() ), RFConf ( conffile.c_str() )

RFProcessManager::RFProcessManager(char* nodename)
{
  // Create IO pipe for output logging
  if (pipe(m_iopipe) < 0) {
    perror("pipe");
    m_iopipe[0] = -1; m_iopipe[1] = -1;
  }
}

RFProcessManager::~RFProcessManager()
{
}

void RFProcessManager::signal_handler(int num)
{
}


int RFProcessManager::Execute(char* scr, int nargs, char** args)
{

  printf("RFProcessManager : Execute : scr=%s, nargs=%d\n", scr, nargs);
  // Fork processes
  pid_t pid = fork();
  if (pid == (pid_t) 0) {   // In case of forked process
    char argbuf[10][256];
    char* argval[10];
    int nargval = 0;
    /// Fill arg0
    strcpy(argbuf[0], scr);
    argval[nargval++] = argbuf[0];
    // Fill argbuf
    for (int i = 0; i < nargs; i++) {
      strcpy(argbuf[nargval], args[i]);
      argval[nargval] = argbuf[nargval];
      nargval++;
      //      printf ( "argval[%d] = %s\n", i+1, argval[i+1] );
    }
    argval[nargval] = NULL;

    // Connect stderr/out to write pipe[1], which is read by parent process thru pipe[0]
    close(1);
    dup2(m_iopipe[1], 1);    // redirect stderr to pipe 1
    close(2);
    dup2(m_iopipe[1], 2);    // redirect stdout to pipe 1
    close(m_iopipe[0]);  // Close read pipe in child process

    // execute script in forked process by execvp()
    printf("RFProcessManager : invoking %s\n", scr);
    for (int i = 0; i < 10; i++) {
      if (argval[i] == NULL) break;
      //      printf ( "argval[%d] = %s\n", i, argval[i] );
    }
    fflush(stdout);
    fflush(stderr);
    int stexe = execvp(scr, argval);
    perror("execvp!!");
    exit(-1);
  }
  // Parent process (pid>0 : success, pid<0 : error )
  printf("RFProcessManager : forked. pid=%d\n", pid);

  return pid;
}

int RFProcessManager::Execute(char* scr,
                              char* arg0, char* arg1, char* arg2, char* arg3,
                              char* arg4, char* arg5, char* arg6, char* arg7)
{
  char* arglist[8];
  int narg = 0;
  if (arg0 != NULL) {
    arglist[0] = arg0;
    narg++;
  }
  if (arg1 != NULL) {
    arglist[1] = arg1;
    narg++;
  }
  if (arg2 != NULL) {
    arglist[2] = arg2;
    narg++;
  }
  if (arg3 != NULL) {
    arglist[3] = arg3;
    narg++;
  }
  if (arg4 != NULL) {
    arglist[4] = arg4;
    narg++;
  }
  if (arg5 != NULL) {
    arglist[5] = arg5;
    narg++;
  }
  if (arg6 != NULL) {
    arglist[6] = arg6;
    narg++;
  }
  if (arg7 != NULL) {
    arglist[7] = arg7;
    narg++;
  }
  return Execute(scr, narg, arglist);
}

int RFProcessManager::CheckOutput()
{
  // Setup select parameter
  fd_set fdset;
  FD_ZERO(&fdset);
  if (m_iopipe[0] > 0) FD_SET(m_iopipe[0], &fdset);
  int highest = m_iopipe[0];

  // Time out parameter
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 1; // omajinai

  // Fetch fd
  int nfd;
  for (;;) {
    if ((nfd = select(highest + 1, &fdset, NULL, NULL, &tv)) < 0) {
      if (errno == EINTR) continue;
    } else {
      break;
    }
    sleep(1);
  }
  // Return nfd
  //  time_t now = time ( NULL );
  //  printf ( "[%s] CheckOutput : nfd = %d\n", ctime(&now), nfd );
  //  fflush ( stdout );
  return nfd;
}

int RFProcessManager::GetFd()
{
  return m_iopipe[0];
}








