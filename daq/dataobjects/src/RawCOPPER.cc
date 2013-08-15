//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawCOPPER.h"

using namespace std;
using namespace Belle2;

ClassImp(RawCOPPER);

RawCOPPER::RawCOPPER()
{
  m_nwords = 0;
  m_buffer = NULL;
  m_allocated = false;
}

RawCOPPER::~RawCOPPER()
{
  if (m_allocated) delete[] m_buffer;
}

void RawCOPPER::Copy(int* bufin, int nwords)
{
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = new int[m_nwords];
  memcpy((char*)m_buffer, (char*)bufin, m_nwords * sizeof(int));
  m_allocated = true;
}

RawHeader* RawCOPPER::GetRawHeader()
{
  return &m_header;
}

RawTrailer* RawCOPPER::GetRawTrailer()
{
  return &m_trailer;
}


int RawCOPPER::GetBodyNwords()
{
  return m_nwords;
}

int RawCOPPER::Size()
{
  return m_nwords;
}

int* RawCOPPER::AllocateBuffer(int nwords)
{
  m_nwords = nwords;
  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[nwords];
  m_allocated = true;
  return m_buffer;
}

int* RawCOPPER::GetBuffer()
{
  return m_buffer;
}


void RawCOPPER::SetBuffer(int* bufin, int nwords, int malloc_flag)
{
  if (m_allocated) delete[] m_buffer;

  if (malloc_flag == 0) {
    m_allocated = false;
  } else {
    m_allocated = true;
  }
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = bufin;

  //
  // Assign header and trailer
  //
  m_header.SetBuffer(&(bufin[ 0 ]));
  m_trailer.SetBuffer(&(bufin[ m_nwords - m_trailer.GetTrlNwords() ]));

}




int RawCOPPER::GetCopperNodeId()
{
  return 0;
}

unsigned int RawCOPPER::GetCoppereveNo()
{
  return m_buffer[ POS_EVE_NUM_COPPER ];
}

int RawCOPPER::GetSubsysId()
{
  unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID ];
  unsigned int crate = m_buffer[ POS_CRATE_ID ];
  unsigned int slot  = m_buffer[ POS_SLOT_ID ];

  return
    ((subsys << 16) & 0xFFFF0000) |
    ((crate << 8) & 0x0000FF00) |
    (slot & 0x000000FF);
}

int RawCOPPER::GetNumB2lBlock()
{
  int cnt = 0;
  if (m_buffer[ POS_CH_A_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_B_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_C_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_D_DATA_LENGTH ] > 0) cnt++;
  return cnt;
}

int RawCOPPER::Offset1stB2lWoRawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ];
}

int RawCOPPER::Offset2ndB2lWoRawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ];
}

int RawCOPPER::Offset3rdB2lWoRawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ]
    + m_buffer[ POS_CH_B_DATA_LENGTH ];
}

int RawCOPPER::Offset4thB2lWoRawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ]
    + m_buffer[ POS_CH_B_DATA_LENGTH ]
    + m_buffer[ POS_CH_C_DATA_LENGTH ];
}
