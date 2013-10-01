//+
// File : RawDataBlock.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawDataBlock.h"

using namespace std;
using namespace Belle2;

ClassImp(RawDataBlock);

RawDataBlock::RawDataBlock()
{
  m_nwords = 0;
  m_buffer = NULL;
  m_use_prealloc_buf = false;
}

RawDataBlock::~RawDataBlock()
{
  if (!m_use_prealloc_buf && m_buffer != NULL) {
    delete[] m_buffer;
  }
}


int RawDataBlock::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("RawDataBlock buffer is not available.\n");
    exit(1);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    printf("Invalid COPPER block No. (%d : max %d ) is specified. Exiting... ", n, (m_num_events * m_num_nodes));
    exit(1);
  }

  int pos_nwords = 0;
  for (int i = 1; i <= n ; i++) {
    if (m_buffer[ pos_nwords ] <= 0) {
      printf("length of this data block is strange ( %d words ). Maybe data is corrupted or RawHeader info has not been filled yet. Exiting...", m_buffer[ pos_nwords ]);
      sleep(1234567);
      exit(1);
    } else {
      pos_nwords +=      m_buffer[ pos_nwords ];
    }
    if (pos_nwords >= m_nwords) {
      printf("value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...", pos_nwords, m_nwords);
      exit(1);
    }
  }
  return pos_nwords;

}


int RawDataBlock::TotalBufNwords()
{
  return m_nwords;
}


int RawDataBlock::GetBlockNwords(int n)
{
  int size;
  //  printf("aaaaaaaaaaaaa %d %d\n", n, m_num_events * m_num_nodes );
  if (n == (m_num_events * m_num_nodes) - 1) {
    size =  m_nwords - GetBufferPos(n);
  } else {
    size = GetBufferPos(n + 1) - GetBufferPos(n);
  }
  return size;
}


int* RawDataBlock::GetWholeBuffer()
{
  return m_buffer;
}

int* RawDataBlock::GetBuffer(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}


void RawDataBlock::SetBuffer(int* bufin, int nwords, int malloc_flag, int num_events, int num_nodes)
{
  if (bufin == NULL) {
    printf("bufin is NULL. Exting...\n");
    exit(1);
  }

  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (malloc_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  // Set length at the first word of the buffer


  //
  // Assign header and trailer
  //
//   m_header.SetBuffer(&(bufin[ 0 ]));
//   m_trailer.SetBuffer(&(bufin[ m_nwords - m_trailer.GetTrlNwords() ]));

}
