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
  m_nwords = 1;
  m_buffer = new int[1]; // Allocate dummy buffer
  m_allocated = true;
}

RawCOPPER::RawCOPPER(int* buffer, int nwords)
{
  //  m_nwords = buffer[0];
  m_nwords = nwords;
  m_buffer = buffer;
  m_allocated = false;
}

RawCOPPER::~RawCOPPER()
{
  if (m_allocated) delete[] m_buffer;
}

void RawCOPPER::copy(int* bufin, int nwords)
{
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = new int[m_nwords];
  memcpy((char*)m_buffer, (char*)bufin, m_nwords * sizeof(int));
  m_allocated = true;
}

int RawCOPPER::get_body_nwords()
{
  return m_nwords;
}

int RawCOPPER::size()
{
  return m_nwords;
}

int* RawCOPPER::allocate_buffer(int nwords)
{
  m_nwords = nwords;
  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[nwords];
  m_allocated = true;
  return m_buffer;
}

int* RawCOPPER::buffer()
{
  return m_buffer;
}

void RawCOPPER::buffer(int* bufin, int nwords)
{
  if (m_allocated) delete[] m_buffer;
  m_allocated = false;

  m_nwords = nwords;
  m_buffer = bufin;
}

void RawCOPPER::buffer(int* bufin, int nwords, int malloc_flag)
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
}

int* RawCOPPER::header(void)
{
  return m_buffer;
}

void RawCOPPER::header(int* hdr)
{
  memcpy((char*)m_buffer, (char*)hdr, HEADER_SIZE * 4);
}

int* RawCOPPER::data(void)
{
  int* ptr = m_buffer + HEADER_SIZE;
  return ptr;
}

void RawCOPPER::data(int nwords, int* data)
{
  memcpy((char*)(m_buffer + HEADER_SIZE), data, nwords * sizeof(int));
  m_buffer[2] = nwords;
  //  m_buffer[1] = HEADER_SIZE;
  m_buffer[0] = HEADER_SIZE + nwords;
  m_nwords = m_buffer[0];
}



int RawCOPPER::get_copper_node_id()
{
  return 0;
}

unsigned int RawCOPPER::get_coppereve_no()
{
  return m_buffer[ POS_EVE_NUM_COPPER ];
}

int RawCOPPER::get_subsys_id()
{
  unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID ];
  unsigned int crate = m_buffer[ POS_CRATE_ID ];
  unsigned int slot  = m_buffer[ POS_SLOT_ID ];

  return
    ((subsys << 16) & 0xFFFF0000) |
    ((crate << 8) & 0x0000FF00) |
    (slot & 0x000000FF);
}

int RawCOPPER::get_num_b2l_block()
{
  int cnt = 0;
  if (m_buffer[ POS_CH_A_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_B_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_C_DATA_LENGTH ] > 0) cnt++;
  if (m_buffer[ POS_CH_D_DATA_LENGTH ] > 0) cnt++;
  return cnt;
}

int RawCOPPER::offset_1st_b2l_wo_rawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ];
}

int RawCOPPER::offset_2nd_b2l_wo_rawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ];
}

int RawCOPPER::offset_3rd_b2l_wo_rawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ]
    + m_buffer[ POS_CH_B_DATA_LENGTH ];
}

int RawCOPPER::offset_4th_b2l_wo_rawhdr()
{
  return
    m_buffer[ SIZE_COPPER_HEADER ]
    + m_buffer[ POS_CH_A_DATA_LENGTH ]
    + m_buffer[ POS_CH_B_DATA_LENGTH ]
    + m_buffer[ POS_CH_C_DATA_LENGTH ];
}
