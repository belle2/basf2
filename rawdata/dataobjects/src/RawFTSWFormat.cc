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
          "[FATAL] This function is not supported in the version of RawFTSW format that you're using. n=%d tvsec=%d : %s %s %d: Exiting...\n",
          n, tv->tv_sec, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return;

}
