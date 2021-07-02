/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

// The parameter n is unused: let's disable the compiler warning for this function.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void RawFTSWFormat::GetPCTimeVal(int n, struct timeval* tv)
{
  // According to Nakao-san's comment at DAQ meeting on Feb.28, 2020. If one calls this function for older data, just returning 0 values is fine.
  // No need to either end up with FATAL message or issue any WARNING messages (Actually, WARNING messages would be issued in every event when implemented.)
  tv->tv_sec = 0;
  tv->tv_usec = 0;

  return;
}

#pragma GCC diagnostic pop
