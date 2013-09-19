//+
// File : RawROPC.cc
// Description : Module to handle raw data on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 16 - Aug - 2013
//-

#include "daq/dataobjects/RawROPC.h"

using namespace std;
using namespace Belle2;

ClassImp(RawROPC);

RawROPC::RawROPC()
{
  m_nwords = 0;
  m_buffer = NULL;
  m_allocated = false;
}

RawROPC::~RawROPC()
{
  if (m_allocated) delete[] m_buffer;
}

void RawROPC::Copy(int* bufin, int nwords)
{
  //  m_nwords = bufin[0];
  m_nwords = nwords;

  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[m_nwords];

  memcpy((char*)m_buffer, (char*)bufin, m_nwords * sizeof(int));
  m_allocated = true;
}

SendHeader* RawROPC::GetSendHeader()
{
  return &m_header;
}

SendTrailer* RawROPC::GetSendTrailer()
{
  return &m_trailer;
}


int RawROPC::GetBodyNwords()
{
  return m_nwords;
}

int RawROPC::Size()
{
  return m_nwords;
}

int* RawROPC::AllocateBuffer(int nwords)
{
  m_nwords = nwords;
  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[nwords];
  m_allocated = true;
  return m_buffer;
}

int* RawROPC::GetBuffer()
{
  return m_buffer;
}


void RawROPC::SetBuffer(int* bufin, int nwords, int malloc_flag)
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
