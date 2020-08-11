//+
// File : ErrorLog.h
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-
//#include <framework/logging/Logger.h>
#include <daq/rawdata/CprErrorMessage.h>
// #include <time.h>
#include <syslog.h>
// #include <framework/core/Module.h>
#include <rawdata/switch_basf2_standalone.h>

using namespace std;
using namespace Belle2;

//REG_MODULE(CprErrorMessage)
// CprErrorMessage::CprErrorMessage( const char* entry_name )
// {
//   //  openlog( entry_name, LOG_PERROR, LOG_LOCAL0 );
// }

CprErrorMessage::CprErrorMessage()
{
  openlog("", LOG_PERROR , LOG_LOCAL0);
}

CprErrorMessage::~CprErrorMessage()
{
  closelog();
}


void CprErrorMessage::PrintError(const int shmflag, RunInfoBuffer* nsm_status, string err_str)
{
  if (shmflag > 0) {
    nsm_status->reportError(RunInfoBuffer::CPRFIFO_FULL);//need to implement error flag
  }
  PrintError(err_str.c_str());
}

void CprErrorMessage::PrintError(const int shmflag, RunInfoBuffer* nsm_status, char* err_message,
                                 const char* file, const char* func_name, const int line)
{
  string err_str = err_message;
  if (shmflag > 0) {
    nsm_status->reportError(RunInfoBuffer::CPRFIFO_FULL);//need to implement error flag
  }
  PrintError(err_message, file, func_name, line);
}


void CprErrorMessage::PrintError(char* err_message, const char* file, const char* func_name, const int line)
{
  char err_buf[500];
  sprintf(err_buf, "%s : %s %s %d", err_message, file, func_name, line);
  //  fprintf( stderr, "[FATAL] %s", err_message);
  printf("%s", err_buf);  fflush(stdout);
  //  B2FATAL(err_buf);
  //  printf("AL] %s\n", err_buf);
  exit(1);

//   time_t     current;
//   time(&current);
//   printf("\033[31m");
//   perror("[ERROR] 0: ");
//   printf("[ERROR] 1: %s", ctime(&current));
//   printf("[ERROR] 2: %s\n", err_message);
//   printf("[ERROR] 3: [file] %s [Line] %d\n", file, line);
//   printf("[ERROR] 4: [function] %s\n", func_name);
//   printf("\033[0m");
//   fflush(stdout);
//   errmsg(LOG_LOCAL0|LOG_ERR,
//   "CRITICAL : %s : init_shm() failed to get shmhead ( %p )\n",
//   __PRETTY_FUNCTION__, shmhead );

  return;
}



void CprErrorMessage::PrintError(const char* err_message)
{
  printf("[DEBUG] %s", err_message);
  fflush(stdout);
  //  B2FATAL(err_message);
  printf("[FATAL] %s\n", err_message); exit(1);

//   printf("\033[31m");
//   printf("\033[47m");
//   //  B2FATAL(err_message);
//   perror("[ERROR] 0: ");
//   printf("[ERROR] 1: %s", ctime(&current));
//   printf("[ERROR] 2: %s\n", err_message);
//   printf("\033[0m");
//   printf("\033[40m");
//   fflush(stdout);
  return;
}
