/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

RawDataBlock::RawDataBlock()
{
  m_access_dblk = new RawDataBlockFormat;

  m_nwords = 0;
  m_use_prealloc_buf = 0;
  m_buffer = NULL;
  m_num_nodes = 0;
  m_num_events = 0;
}

RawDataBlock::~RawDataBlock()
{
  if (m_access_dblk != NULL) delete m_access_dblk;
  m_access_dblk = NULL;

  if (!m_use_prealloc_buf && m_buffer != NULL) {
    delete[] m_buffer;
  }
}

void RawDataBlock::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
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
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  // Set to access class
  m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);

  // Set length at the first word of the buffer
  //
  // Assign header and trailer
  //
  //   m_header.SetBuffer(&(bufin[ 0 ]));
  //   m_trailer.SetBuffer(&(bufin[ m_nwords - m_trailer.GetTrlNwords() ]));

}
