/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dataobjects/SendTrailer.h"

#include <cstring>

using namespace std;
using namespace Belle2;

//ClassImp(SendTrailer);

SendTrailer::SendTrailer()
{
  Initialize();
  //  cout << "SendTrailer NULL constructor" << endl;
}

SendTrailer::SendTrailer(int* buffer)
{
  memcpy(m_buffer, buffer, SENDTRL_NWORDS * sizeof(int));
}

SendTrailer::~SendTrailer()
{
}

int* SendTrailer::GetBuffer()
{
  return m_buffer;
}

void SendTrailer::SetBuffer(int* bufin)
{
  memcpy(m_buffer, bufin, SENDTRL_NWORDS * sizeof(int));
}

void SendTrailer::Initialize()
{
  memset(m_buffer, 0, SENDTRL_NWORDS * sizeof(int));
  SetMagicWord();
}


void SendTrailer::SetChksum(int chksum)
{
  m_buffer[ POS_CHKSUM ] = chksum;
}

void SendTrailer::SetMagicWord()
{
  m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_SEND_TRAILER;
}

int SendTrailer::GetMagicWord()
{
  return m_buffer[ POS_TERM_WORD ];
}

int SendTrailer::GetTrlNwords()
{
  return SENDTRL_NWORDS;
}
