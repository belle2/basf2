//+
// File : RawFTSW.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawFTSW.h"
#include <daq/rawdata/modules/DAQConsts.h>

using namespace std;
using namespace Belle2;

ClassImp(RawFTSW);

RawFTSW::RawFTSW()
{
}

RawFTSW::~RawFTSW()
{
}


int RawFTSW::GetNwords(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NWORDS ];
}

int RawFTSW::GetNwordsHeader(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_HDR_NWORDS ];
}


int RawFTSW::GetFTSWNodeID(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NODE_ID ];
}

// unsigned int RawFTSW::GetEveNo(int n){


// }
