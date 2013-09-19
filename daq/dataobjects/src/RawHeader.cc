//+
// File : RawHeader.cc
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(RawHeader);

RawHeader::RawHeader()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "RawHeader NULL constructor" << endl;
}

RawHeader::RawHeader(int* buffer)
{
  m_buffer = buffer;
}

RawHeader::~RawHeader()
{
}

int* RawHeader::GetBuffer()
{
  return m_buffer;
}

void RawHeader::SetBuffer(int* bufin)
{
  m_buffer = bufin;
}

void RawHeader::CheckBuffer()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }
}


void RawHeader::Initialize()
{
  CheckBuffer();
  memset(m_buffer, 0, sizeof(int)*RAWHEADER_NWORDS);
  m_buffer[ POS_HDR_NWORDS ] = RAWHEADER_NWORDS;
  m_buffer[ POS_NUM_NODES ] = 0;
  m_buffer[ POS_TERM_HEADER ] = MAGIC_WORD_TERM_HEADER;
}


void RawHeader::SetNwords(int nwords)
{
  CheckBuffer();
  m_buffer[ POS_NWORDS ] = nwords;

}

void RawHeader::SetExpNo(int exp_no)
{
  CheckBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0x003FFFFF) | ((exp_no << 22) & 0xFFC00000);
}

void RawHeader::SetRunNo(int run_no)
{
  CheckBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0xFFC00000) | (run_no & 0x003FFFFF);
}

void RawHeader::SetEveNo(unsigned int eve_no)
{
  CheckBuffer();
  m_buffer[ POS_EVE_NO ] = eve_no;
}
void RawHeader::SetSubsysId(int subsys_id)
{
  CheckBuffer();
  m_buffer[ POS_SUBSYS_ID ] = subsys_id;
}
void RawHeader::SetDataType(int data_type)
{
  CheckBuffer();
  m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
    (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
}

void RawHeader::SetTruncMask(int trunc_mask)
{
  CheckBuffer();
  m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
}

void RawHeader::SetB2LFEEHdrPart(unsigned int word1, unsigned int word2)
{
  m_buffer[ POS_HSLB_1 ] = word1;
  m_buffer[ POS_HSLB_2 ] = word2;

}


void RawHeader::SetOffset1stFINNESSE(int offset_1st_FINNESSE)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_1ST_FINNESSE ] = offset_1st_FINNESSE;
}

void RawHeader::SetOffset2ndFINNESSE(int offset_2nd_FINNESSE)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_2ND_FINNESSE ] = offset_2nd_FINNESSE;
}
void RawHeader::SetOffset3rdFINNESSE(int offset_3rd_FINNESSE)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_3RD_FINNESSE ] = offset_3rd_FINNESSE;
}
void RawHeader::SetOffset4thFINNESSE(int offset_4th_FINNESSE)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_4TH_FINNESSE ] = offset_4th_FINNESSE;
}

int RawHeader::AddNodeInfo(int node_id)
{
  CheckBuffer();
  // When the number of total nodes exceeds NUM_MAX_NODES
  if (GetNumNodes() > NUM_MAX_NODES) {
    m_buffer[ POS_NUM_NODES ]++;
    return -1;
  }

  m_buffer[ POS_NODES_1 + m_buffer[ POS_NUM_NODES ] ] = node_id;
  m_buffer[ POS_NUM_NODES ]++;
  return 0;
}

int RawHeader::GetNwords()
{
  CheckBuffer();
  return m_buffer[ POS_NWORDS ];
}

int RawHeader::GetHdrNwords()
{
  CheckBuffer();
  return m_buffer[ POS_HDR_NWORDS ];
}

int RawHeader::GetExpNo()
{
  CheckBuffer();
  return ((m_buffer[ POS_EXP_RUN_NO ] >> 22) & 0x000003FF);
}

int RawHeader::GetRunNo()
{
  CheckBuffer();
  return (m_buffer[ POS_EXP_RUN_NO ]  & 0x003FFFFF);
}

unsigned int RawHeader::GetEveNo()
{
  CheckBuffer();
  return m_buffer[ POS_EVE_NO ];
}

int RawHeader::GetSubsysId()
{
  CheckBuffer();
  return m_buffer[ POS_SUBSYS_ID ];
}

int RawHeader::GetDataType()
{
  CheckBuffer();
  return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
}

int RawHeader::GetTruncMask()
{
  CheckBuffer();
  return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
}


int RawHeader::GetOffset1stFINNESSE()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_1ST_FINNESSE ];
}

int RawHeader::GetOffset2ndFINNESSE()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_2ND_FINNESSE ];
}

int RawHeader::GetOffset3rdFINNESSE()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_3RD_FINNESSE ];
}

int RawHeader::GetOffset4thFINNESSE()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_4TH_FINNESSE ];
}

int RawHeader::GetNumNodes()
{
  CheckBuffer();
  return m_buffer[ POS_NUM_NODES ];
}

int RawHeader::GetNodeInfo(int node_no, int* node_id)
{
  CheckBuffer();
  if (node_no >= GetNumNodes()) {
    return -1;
  }
  *node_id = m_buffer[ POS_NODES_1 + node_no ];

  return 0;
}


unsigned int RawHeader::GetMagicWordEntireHeader()
{
  CheckBuffer();
  return m_buffer[ POS_TERM_HEADER ];
}
