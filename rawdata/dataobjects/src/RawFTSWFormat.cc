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
  string err_str = err_buf;
  throw (err_str);
  return -1;
}

