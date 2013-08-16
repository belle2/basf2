//+
// File : ROPCHeader.cc
// Description : Module to handle ROPCHeader attached on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 16 - Aug - 2013
//-

#include "daq/dataobjects/ROPCHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(ROPCHeader);

ROPCHeader::ROPCHeader()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "ROPCHeader NULL constructor" << endl;
}

ROPCHeader::ROPCHeader(int* buffer)
{
  m_buffer = buffer;
}

ROPCHeader::~ROPCHeader()
{
}

int* ROPCHeader::GetBuffer()
{
  return m_buffer;
}

void ROPCHeader::SetBuffer(int* bufin)
{
  m_buffer = bufin;
}

void ROPCHeader::CheckBuffer()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }
}


void ROPCHeader::Initialize()
{
  CheckBuffer();
  memset(m_buffer, 0, sizeof(int)*ROPCHEADER_NWORDS);
  m_buffer[ POS_HDR_NWORDS ] = ROPCHEADER_NWORDS;
}


void ROPCHeader::SetNwords(int nwords)
{
  CheckBuffer();
  m_buffer[ POS_NWORDS ] = nwords;

}

void ROPCHeader::SetExpNo(int exp_no)
{
  CheckBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0x003FFFFF) | ((exp_no << 22) & 0xFFC00000);
}

void ROPCHeader::SetRunNo(int run_no)
{
  CheckBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0xFFC00000) | (run_no & 0x003FFFFF);
}

void ROPCHeader::SetEveNo(unsigned int eve_no)
{
  CheckBuffer();
  m_buffer[ POS_EVE_NO ] = eve_no;
}

void ROPCHeader::SetROPCNodeId(int ropc_node_id)
{
  CheckBuffer();
  m_buffer[ POS_ROPC_NODE_ID ] = ropc_node_id;
}

int ROPCHeader::GetNwords()
{
  CheckBuffer();
  return m_buffer[ POS_NWORDS ];
}

int ROPCHeader::GetHdrNwords()
{
  CheckBuffer();
  return m_buffer[ POS_HDR_NWORDS ];
}

int ROPCHeader::GetExpNo()
{
  CheckBuffer();
  return ((m_buffer[ POS_EXP_RUN_NO ] >> 22) & 0x000003FF);
}

int ROPCHeader::GetRunNo()
{
  CheckBuffer();
  return (m_buffer[ POS_EXP_RUN_NO ]  & 0x003FFFFF);
}

unsigned int ROPCHeader::GetEveNo()
{
  CheckBuffer();
  return m_buffer[ POS_EVE_NO ];
}

int ROPCHeader::GetROPCNodeId()
{
  CheckBuffer();
  return m_buffer[ POS_ROPC_NODE_ID ];
}
