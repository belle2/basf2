//+
// File : RawFTSW.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawFTSW.h"

using namespace std;
using namespace Belle2;

ClassImp(RawFTSW);

RawFTSW::RawFTSW()
{
}

RawFTSW::~RawFTSW()
{
}

void RawFTSW::SetFTSWHeader(int n)
{
  if (n >= GetNumEntries()) {
    printf("Invalid FTSW block No. (%d : max %d ) is specified. Exiting... ", n, (m_num_events * m_num_nodes));
    exit(1);
  }
  m_FTSW_header.SetBuffer(GetBuffer(n));
  return;

}
