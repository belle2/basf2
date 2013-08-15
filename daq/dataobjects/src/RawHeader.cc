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
  m_buffer[ POS_TERM_FIXED_PART ] = MAGIC_WORD_TERM_FIXED_PART;
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
  m_buffer[ POS_DATA_TYPE ] = data_type;
}
void RawHeader::SetTruncMask(int trunc_mask)
{
  CheckBuffer();
  m_buffer[ POS_TRUNC_MASK ] = trunc_mask;
}

void RawHeader::SetNumB2lBlock(int num_b2l_block)
{
  CheckBuffer();
  m_buffer[ POS_NUM_B2L_BLOCK ] = num_b2l_block;
}

void RawHeader::SetOffset1stB2l(int offset_1st_b2l)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_1ST_B2L ] = offset_1st_b2l;
}

void RawHeader::SetOffset2ndB2l(int offset_2nd_b2l)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_2ND_B2L ] = offset_2nd_b2l;
}
void RawHeader::SetOffset3rdB2l(int offset_3rd_b2l)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_3RD_B2L ] = offset_3rd_b2l;
}
void RawHeader::SetOffset4thB2l(int offset_4th_b2l)
{
  CheckBuffer();
  m_buffer[ POS_OFFSET_4TH_B2L ] = offset_4th_b2l;
}

int RawHeader::AddNodeInfo(int node_id)
{
  CheckBuffer();
  // When the number of total nodes exceeds NUM_MAX_NODES
  if (GetNumNodes() > NUM_MAX_NODES) {
    m_buffer[ POS_NUM_NODES ]++;
    return -1;
  }

  m_buffer[ POS_NODES_0 + m_buffer[ POS_NUM_NODES ] ] = node_id;
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
  return m_buffer[ POS_DATA_TYPE ];
}

int RawHeader::GetTruncMask()
{
  CheckBuffer();
  return m_buffer[ POS_TRUNC_MASK ];
}

int RawHeader::GetNumB2lBlock()
{
  CheckBuffer();
  return m_buffer[ POS_NUM_B2L_BLOCK ];
}

int RawHeader::GetOffset1stB2l()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_1ST_B2L ];
}

int RawHeader::GetOffset2ndB2l()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_2ND_B2L ];
}

int RawHeader::GetOffset3rdB2l()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_3RD_B2L ];
}

int RawHeader::GetOffset4thB2l()
{
  CheckBuffer();
  return m_buffer[ POS_OFFSET_4TH_B2L ];
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
  *node_id = m_buffer[ POS_NODES_0 + node_no ];

  return 0;
}

unsigned int RawHeader::GetMagicWordFixedPart()
{
  CheckBuffer();
  return m_buffer[ POS_TERM_FIXED_PART ];
}

unsigned int RawHeader::GetMagicWordEntireHeader()
{
  CheckBuffer();
  return m_buffer[ POS_TERM_HEADER ];
}

