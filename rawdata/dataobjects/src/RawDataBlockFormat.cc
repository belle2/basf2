/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/dataobjects/RawDataBlockFormat.h>

using namespace std;
using namespace Belle2;

RawDataBlockFormat::RawDataBlockFormat()
{
  m_nwords = 0;
  m_use_prealloc_buf = 0;
  m_buffer = NULL;
  m_num_nodes = 0;
  m_num_events = 0;
}

RawDataBlockFormat::~RawDataBlockFormat()
{
  if (!m_use_prealloc_buf && m_buffer != NULL) {
    delete[] m_buffer;
  }
}

int RawDataBlockFormat::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] RawPacket buffer(%p) is not available or length(%d) is not set.\n %s %s %d\n",
            m_buffer, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Invalid COPPER block No. (%d : max %d ) is specified. Exiting... \n %s %s %d\n",
            n, (m_num_events * m_num_nodes), __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  int pos_nwords = 0;
  for (int i = 1; i <= n ; i++) {
    if (m_buffer[ pos_nwords ] <= 0) {
      char err_buf[500];
      sprintf(err_buf,
              "[FATAL] ERROR_EVENT : length of this data block is strange ( %d words ). Maybe data is corrupted or RawHeader info has not been filled yet. Exiting...",
              m_buffer[ pos_nwords ]);
      printf("%s", err_buf);
      B2FATAL(err_buf);
    } else {
      pos_nwords +=      m_buffer[ pos_nwords ];
    }
    if (pos_nwords >= m_nwords) {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] ERROR_EVENT : value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...\n %s %s %d\n",
              pos_nwords, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      B2FATAL(err_buf); // to reduce multiple error messages
    }
  }
  return pos_nwords;

}


int RawDataBlockFormat::CheckFTSWID(int n)
{
  int pos = POS_NODE_ID;
  if (m_buffer[ GetBufferPos(n) + TEMP_POS_NWORDS_HEADER ] == OLD_FTSW_NWORDS_HEADER) {
    pos =  POS_FTSW_ID_OLD;
  }

  if ((m_buffer[ GetBufferPos(n) + pos ] & 0xffffff00) == 0x54544400) {     // "TTD" + format version ( 0x20=DESY, 0x31=2018/7/11)
    return 1;
  } else {
    return 0;
  }
}


int RawDataBlockFormat::CheckTLUID(int n)
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

int RawDataBlockFormat::TotalBufNwords()
{
  return m_nwords;
}


int RawDataBlockFormat::GetBlockNwords(int n)
{
  int size;
  if (n == (m_num_events * m_num_nodes) - 1) {
    size =  m_nwords - GetBufferPos(n);
  } else {
    size = GetBufferPos(n + 1) - GetBufferPos(n);
  }
  return size;
}


int* RawDataBlockFormat::GetWholeBuffer()
{
  return m_buffer;
}

int* RawDataBlockFormat::GetBuffer(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}


void RawDataBlockFormat::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
{

  if (bufin == NULL) {
    char err_buf[500];
    sprintf(err_buf, "[DEBUG] bufin is NULL. Exting...\n");
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (delete_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }

  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  return;

}



void RawDataBlockFormat::PrintData(int* buf, int nwords)
{
  printf("[DEBUG] ");
  for (int i = 0; i < nwords; i++) {
    printf("%.8x ", buf[ i ]);
    if (i % 10 == 9) printf("\n[DEBUG] ");
  }
  printf("\n[DEBUG] ");
  printf("\n");
  return;
}

void RawDataBlockFormat::CopyBlock(int n, int* buf_to)
{
  memcpy(buf_to, GetBuffer(n), GetBlockNwords(n) * sizeof(int));
  return;
}
