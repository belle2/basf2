//+
// File : RawHeader.cc
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader.h>


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

void RawHeader::CheckSetBuffer()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }
}

void RawHeader::CheckGetBuffer()
{
  if (m_buffer == NULL || m_buffer[ POS_TERM_HEADER ] != MAGIC_WORD_TERM_HEADER) {
    printf("m_buffer is NULL(%p) or magic word is invalid(0x%x). Data is corrupted or header info has not yet filled. Exiting...",
           m_buffer,
           m_buffer[ POS_TERM_HEADER ]
          );
    exit(1);
  }
}

//
// Set info.
//
void RawHeader::Initialize()
{
  CheckSetBuffer();
  memset(m_buffer, 0, sizeof(int)*RAWHEADER_NWORDS);
  m_buffer[ POS_HDR_NWORDS ] = RAWHEADER_NWORDS;
  m_buffer[ POS_NUM_NODES ] = 0;

}

void RawHeader::SetMagicWordEntireHeader()
{
  m_buffer[ POS_TERM_HEADER ] = MAGIC_WORD_TERM_HEADER;
}

void RawHeader::SetNwords(int nwords)
{
  CheckSetBuffer();
  m_buffer[ POS_NWORDS ] = nwords;

}

void RawHeader::SetExpNo(int exp_no)
{
  CheckSetBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0x003FFFFF) | ((exp_no << 22) & 0xFFC00000);
}

void RawHeader::SetRunNo(int run_no)
{
  CheckSetBuffer();
  m_buffer[ POS_EXP_RUN_NO ] = (m_buffer[ POS_EXP_RUN_NO ] & 0xFFC00000) | (run_no & 0x003FFFFF);
}

void RawHeader::SetEveNo(unsigned int eve_no)
{
  CheckSetBuffer();
  m_buffer[ POS_EVE_NO ] = eve_no;
}
void RawHeader::SetSubsysId(int subsys_id)
{
  CheckSetBuffer();
  m_buffer[ POS_SUBSYS_ID ] = subsys_id;
}
void RawHeader::SetDataType(int data_type)
{
  CheckSetBuffer();
  m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
    (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
}

void RawHeader::SetTruncMask(int trunc_mask)
{
  CheckSetBuffer();
  m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
}

void RawHeader::SetB2LFEEHdrPart(unsigned int word1, unsigned int word2)
{
  m_buffer[ POS_HSLB_1 ] = word1;
  m_buffer[ POS_HSLB_2 ] = word2;

}


void RawHeader::SetOffset1stFINESSE(int offset_1st_FINESSE)
{
  CheckSetBuffer();
  m_buffer[ POS_OFFSET_1ST_FINESSE ] = offset_1st_FINESSE;
}

void RawHeader::SetOffset2ndFINESSE(int offset_2nd_FINESSE)
{
  CheckSetBuffer();
  m_buffer[ POS_OFFSET_2ND_FINESSE ] = offset_2nd_FINESSE;
}
void RawHeader::SetOffset3rdFINESSE(int offset_3rd_FINESSE)
{
  CheckSetBuffer();
  m_buffer[ POS_OFFSET_3RD_FINESSE ] = offset_3rd_FINESSE;
}
void RawHeader::SetOffset4thFINESSE(int offset_4th_FINESSE)
{
  CheckSetBuffer();
  m_buffer[ POS_OFFSET_4TH_FINESSE ] = offset_4th_FINESSE;
}

void RawHeader::SetFTSW2Words(int* ftsw_buf)
{
  CheckSetBuffer();
  memcpy(&(m_buffer[ POS_HSLB_1 ]), (char*)ftsw_buf, sizeof(int) * 2);
  return;
}

void RawHeader::SetFTSW2Words(unsigned int word1, unsigned int word2)
{
  CheckSetBuffer();
  m_buffer[ POS_HSLB_1 ] = word1;
  m_buffer[ POS_HSLB_2 ] = word2;
  return;
}


void RawHeader::SetExpRunNumber(int* exprun_buf)
{
  CheckSetBuffer();
  memcpy(&(m_buffer[ POS_EXP_RUN_NO ]), (char*)exprun_buf, sizeof(int) * 1);
  return;
}


int RawHeader::AddNodeInfo(int node_id)
{
  CheckSetBuffer();
  // When the number of total nodes exceeds NUM_MAX_NODES
  if (GetNumNodes() > NUM_MAX_NODES) {
    m_buffer[ POS_NUM_NODES ]++;
    return -1;
  }

  m_buffer[ POS_NODES_1 + m_buffer[ POS_NUM_NODES ] ] = node_id;
  m_buffer[ POS_NUM_NODES ]++;
  return 0;
}

//
// Obtain info
//

int RawHeader::GetNwords()
{
  CheckGetBuffer();
  return m_buffer[ POS_NWORDS ];
}

int RawHeader::GetHdrNwords()
{
  CheckGetBuffer();
  return m_buffer[ POS_HDR_NWORDS ];
}

int RawHeader::GetExpNo()
{
  CheckGetBuffer();
  return ((m_buffer[ POS_EXP_RUN_NO ] >> 22) & 0x000003FF);
}

int RawHeader::GetRunNo()
{
  CheckGetBuffer();
  return (m_buffer[ POS_EXP_RUN_NO ]  & 0x003FFFFF);
}

unsigned int RawHeader::GetEveNo()
{
  CheckGetBuffer();
  return m_buffer[ POS_EVE_NO ];
}

int RawHeader::GetSubsysId()
{
  CheckGetBuffer();
  return m_buffer[ POS_SUBSYS_ID ];
//   unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int crate = m_buffer[ POS_CRATE_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int slot  = m_buffer[ POS_SLOT_ID + tmp_header.RAWHEADER_NWORDS ];
//   return
//     ((subsys << 16) & 0xFFFF0000) |
//     ((crate << 8) & 0x0000FF00) |
//     (slot & 0x000000FF);

}

int RawHeader::GetDataType()
{
  CheckGetBuffer();
  return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
}

int RawHeader::GetTruncMask()
{
  CheckGetBuffer();
  return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
}


int RawHeader::GetOffset1stFINESSE()
{
  CheckGetBuffer();
  return m_buffer[ POS_OFFSET_1ST_FINESSE ];
}

int RawHeader::GetOffset2ndFINESSE()
{
  CheckGetBuffer();
  return m_buffer[ POS_OFFSET_2ND_FINESSE ];
}

int RawHeader::GetOffset3rdFINESSE()
{
  CheckGetBuffer();
  return m_buffer[ POS_OFFSET_3RD_FINESSE ];
}

int RawHeader::GetOffset4thFINESSE()
{
  CheckGetBuffer();
  return m_buffer[ POS_OFFSET_4TH_FINESSE ];
}

int RawHeader::GetNumNodes()
{
  CheckGetBuffer();
  return m_buffer[ POS_NUM_NODES ];
}

int RawHeader::GetNodeInfo(int node_no, int* node_id)
{
  CheckGetBuffer();
  if (node_no >= GetNumNodes()) {
    return -1;
  }
  *node_id = m_buffer[ POS_NODES_1 + node_no ];

  return 0;
}


unsigned int RawHeader::GetMagicWordEntireHeader()
{
  return m_buffer[ POS_TERM_HEADER ];
}
