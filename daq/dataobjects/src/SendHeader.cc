//+
// File : SendHeader.cc
// Description : Module to handle SendHeader attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/SendHeader.h"
#include "rawdata/dataobjects/RawHeader.h"

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


void SendHeader::SetRunNum(int run_num)
{
  unsigned int inv_mask = ~((unsigned int)(RawHeader::RUNNO_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) |
    (((unsigned int)run_num << RawHeader::RUNNO_SHIFT) & RawHeader::RUNNO_MASK);
  return;
}

void SendHeader::SetSubRunNum(int subrun_num)
{
  unsigned int inv_mask = ~((unsigned int)(RawHeader::SUBRUNNO_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) | ((unsigned int)subrun_num & RawHeader::SUBRUNNO_MASK);
  return;
}

void SendHeader::SetExpNum(int exp_num)
{
  unsigned int inv_mask = ~((unsigned int)(RawHeader::EXP_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) |
    (((unsigned int)exp_num << RawHeader::EXP_SHIFT) & RawHeader::EXP_MASK);
  return;
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

int SendHeader::GetRunNum()
{
  return (((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & RawHeader::RUNNO_MASK) >> RawHeader::RUNNO_SHIFT);
}

int SendHeader::GetSubRunNum()
{
  return ((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & RawHeader::SUBRUNNO_MASK);
}

int SendHeader::GetRunNumSubrunNum()
{
  return ((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) &
          (RawHeader::RUNNO_MASK | RawHeader::SUBRUNNO_MASK));
}


int SendHeader::GetExpNum()
{
  return (((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & RawHeader::EXP_MASK) >> RawHeader::EXP_SHIFT);
}


