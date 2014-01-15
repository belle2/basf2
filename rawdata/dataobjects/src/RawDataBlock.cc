//+
// File : RawDataBlock.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawDataBlock.h>

using namespace std;
using namespace Belle2;

ClassImp(RawDataBlock);

RawDataBlock::RawDataBlock()
{
  m_nwords = 0;
  m_use_prealloc_buf = 0;
  m_buffer = NULL;

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
      char err_buf[500];
      sprintf(err_buf, "value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...\n %s %s %d\n",
              pos_nwords, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf;     throw (err_str);
      exit(1);
    }
  }
  return pos_nwords;

}


int RawDataBlock::CheckFTSWID(int n)
{
  int pos = POS_NODE_ID;
  if (m_buffer[ GetBufferPos(n) + TEMP_POS_NWORDS_HEADER ] == OLD_FTSW_NWORDS_HEADER) {
    pos =  POS_FTSW_ID_OLD;
  }

  if (m_buffer[ GetBufferPos(n) + pos ] == 0x54544420) { // "TTD "
    return 1;
  } else {
    return 0;
  }
}


int RawDataBlock::CheckTLUID(int n)
{
  int pos = POS_NODE_ID;
  if (m_buffer[ GetBufferPos(n) + TEMP_POS_NWORDS_HEADER ] == OLD_FTSW_NWORDS_HEADER) {
    pos =  POS_FTSW_ID_OLD;
  }
  if (m_buffer[ GetBufferPos(n) + pos ] == 0x544c5520) { // "TLU "
    return 1;
  } else {
    return 0;
  }
}

int RawDataBlock::TotalBufNwords()
{
  return m_nwords;
}


int RawDataBlock::GetBlockNwords(int n)
{
  int size;
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



void RawDataBlock::PrintData(int* buf, int nwords)
{
  for (int i = 0; i < nwords; i++) {
    printf("%.8x ", buf[ i ]);
    if (i % 10 == 9) printf("\n");
  }
  printf("\n");
  printf("\n");
  return;
}

