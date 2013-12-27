//+
// File : RawDataBlock.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/ErrorMessage.h>
#include <time.h>
#include <syslog.h>

using namespace std;
using namespace Belle2;

ClassImp(ErrorMessage);

// ErrorMessage::ErrorMessage( const char* entry_name )
// {
//   //  openlog( entry_name, LOG_PERROR, LOG_LOCAL0 );
// }

ErrorMessage::ErrorMessage()
{
  openlog("", LOG_PERROR , LOG_LOCAL0);
}

ErrorMessage::~ErrorMessage()
{
  closelog();
}

void ErrorMessage::PrintError(ProcessStatusBuffer* nsm_status, char* err_message, const char* file, const char* func_name, const int line)
{
  string err_str = err_message;
  nsm_status->reportFatal(err_str);
  PrintError(err_message, file, func_name, line);
}


void ErrorMessage::PrintError(char* err_message, const char* file, const char* func_name, const int line)
{
  time_t     current;
  time(&current);

  printf("\033[31m");
  perror("[ERROR] 0: ");
  printf("[ERROR] 1: %s", ctime(&current));
  printf("[ERROR] 2: %s\n", err_message);
  printf("[ERROR] 3: [file] %s [Line] %d\n", file, line);
  printf("[ERROR] 4: [function] %s\n", func_name);
  printf("\033[0m");
  fflush(stdout);
//     errmsg(LOG_LOCAL0|LOG_ERR,
//            "CRITICAL : %s : init_shm() failed to get shmhead ( %p )\n",
//            __PRETTY_FUNCTION__, shmhead );

  return;
}



// void ErrorMessage::PrintError(const std::string err_message, const char* file, const char* func_name, const int line)
// {
//   time_t     current;
//   time(&current);

//   printf("\033[31m");
//   perror("[ERROR] 0: ");
//   printf("[ERROR] 1: %s", ctime(&current));
//   printf("[ERROR] 2: %s\n", err_message.c_str());
//   printf("[ERROR] 3: [file] %s [Line] %d\n", file, line);
//   printf("[ERROR] 4: [function] %s\n", func_name);
//   printf("\033[0m");
//   fflush(stdout);
// //     errmsg(LOG_LOCAL0|LOG_ERR,
// //            "CRITICAL : %s : init_shm() failed to get shmhead ( %p )\n",
// //            __PRETTY_FUNCTION__, shmhead );

//   return;
// }
