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
  SendHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return  hdr.GetTotalNwords();
}

int RawFTSW::GetNwordsHeader(int n)
{
  SendHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetHdrNwords();
}


int RawFTSW::GetFTSWNodeID(int n)
{
  SendHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetNodeID();
}

int RawFTSW::GetTrailerMagic(int n)
{
  SendTrailer trl;
  trl.SetBuffer(GetBuffer(n) + GetNwords(n) - SendTrailer::SENDTRL_NWORDS);
  return trl.GetMagicWord();
}


unsigned int RawFTSW::GetFTSWData1(int n)
{
  return  m_buffer[ GetBufferPos(n) + GetNwordsHeader(n) + POS_FTSW1 ];
}

unsigned int RawFTSW::GetFTSWData2(int n)
{
  return  m_buffer[ GetBufferPos(n) + GetNwordsHeader(n) + POS_FTSW2 ];
}

double RawFTSW::GetEventUnixTime(int n)
{
  SendHeader hdr;
  hdr.SetBuffer(GetBuffer(n));

  int run = hdr.GetRunNum();

  return (double)(sp8test_run_starttime[ run ] & 0xFFFF0000) +
         (double)((m_buffer[ GetBufferPos(n) + GetNwordsHeader(n) + POS_FTSW2 ] >> 16) & 0xFFFF) +
         (double)((m_buffer[ GetBufferPos(n) + GetNwordsHeader(n) + POS_FTSW1 ] >> 4) & 0x7FFFFFF) / 1.27e8;
}


