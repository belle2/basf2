//+
// File : SendHeader.cc
// Description : Module to handle SendHeader attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/SendHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(SendHeader);

SendHeader::SendHeader()
{
  Initialize();
  //  cout << "SendHeader NULL constructor" << endl;
}

SendHeader::SendHeader(int* buffer)
{
  //  cout << "SendHeader constructor with buffer" << endl;
  memcpy(m_buffer, buffer, SENDHDR_NWORDS * sizeof(int));
}

SendHeader::~SendHeader()
{
}

int* SendHeader::GetBuffer()
{
  return m_buffer;
}

void SendHeader::SetBuffer(int* bufin)
{
  memcpy(m_buffer, bufin, SENDHDR_NWORDS * sizeof(int));
}

void SendHeader::Initialize()
{
  memset(m_buffer, 0, sizeof(int)* SENDHDR_NWORDS);
  m_buffer[ POS_HDR_NWORDS ] = SENDHDR_NWORDS;
}

void SendHeader::SetNwords(int total_data_nwords)
{
  m_buffer[ POS_NWORDS ] = total_data_nwords;
}

void SendHeader::SetNumEventsinPacket(int num_events)
{
  m_buffer[ POS_NUM_EVENTS_IN_PACKET ] = num_events;
}


int SendHeader::GetTotalNwords() {  return m_buffer[ POS_NWORDS ];}
int SendHeader::GetHdrNwords() {    return SENDHDR_NWORDS; }
int SendHeader::GetNumEventsinPacket() {  return m_buffer[ POS_NUM_EVENTS_IN_PACKET ]; }



