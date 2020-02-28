//+
// File : RawFTSWFormat.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <rawdata/dataobjects/RawFTSWFormat.h>


using namespace std;
using namespace Belle2;

RawFTSWFormat::RawFTSWFormat()
{
}

RawFTSWFormat::~RawFTSWFormat()
{
}

// This function is only available in RawFTSW ver.1 (for DESY test)
int RawFTSWFormat::Get15bitTLUTag(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

// This function is only available in RawFTSW ver.2
int RawFTSWFormat::GetIsHER(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}



unsigned int RawFTSWFormat::GetTimeSinceLastInjection(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;

}

unsigned int RawFTSWFormat::GetTimeSincePrevTrigger(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

unsigned int RawFTSWFormat::GetBunchNumber(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

unsigned int RawFTSWFormat::GetFrameCount(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

void RawFTSWFormat::GetPCTimeVal(int n, struct timeval* tv)
{
  char err_buf[500];
  sprintf(err_buf,
          "[WARNING] This function is not supported in the version of RawFTSW format that you're using. Just returining tv_sec=0 and tv_usec = 0. ( n= %d) Please check disscussion about this handling at https://stash.desy.de/projects/B2/repos/software/pull-requests/5633/ : %s %s %d: \n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2WARNING(err_buf);
  tv->tv_sec = 0;
  tv->tv_usec = 0;

  return;

}
