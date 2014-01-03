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


// Set Values

void SendHeader::SetNwords(int total_data_nwords)
{
  m_buffer[ POS_NWORDS ] = total_data_nwords;
}

void SendHeader::SetExpRunWord(unsigned int exp_run)
{
  m_buffer[ POS_EXP_RUN_NUM ] = exp_run;
}

void SendHeader::SetNumEventsinPacket(int num_events)
{
  m_buffer[ POS_NUM_EVE_NUM_NODES ] =
    (m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0x0000FFFF) |
    ((num_events << 16) & 0xFFFF0000);
}

void SendHeader::SetNumNodesinPacket(int num_nodes)
{
  m_buffer[ POS_NUM_EVE_NUM_NODES ] =
    (m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0xFFFF0000) |
    (num_nodes & 0x0000FFFF);
}


void SendHeader::SetEventNumber(int eve_num)
{
  m_buffer[ POS_EVE_NUM ] = eve_num;
}


void SendHeader::SetNodeID(int node_id)
{
  m_buffer[ POS_NODE_ID ] = node_id;
}


// Get Values

int SendHeader::GetTotalNwords() {  return m_buffer[ POS_NWORDS ];}
int SendHeader::GetHdrNwords() {    return SENDHDR_NWORDS; }
int SendHeader::GetNumEventsinPacket()
{
  return
    ((m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0xFFFF0000) >> 16) & 0x0000FFFF;

}
int SendHeader::GetNumNodesinPacket() {  return m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0x0000FFFF; }

int SendHeader::GetEventNumber() { return m_buffer[ POS_EVE_NUM ]; }

int SendHeader::GetNodeID() { return m_buffer[ POS_NODE_ID ]; }

int SendHeader::GetRunNum() { return (m_buffer[ POS_EXP_RUN_NUM ] & 0x3FFFFF); }

int SendHeader::GetExpNum() { return ((m_buffer[ POS_EXP_RUN_NUM ] >> 22) & 0x3FF); }


