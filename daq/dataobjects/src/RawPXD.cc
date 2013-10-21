//+
// File : RawPXD.cc
// Description : Module to handle raw data on PXD
//
// Author : Tomoyuki Konno, IPNS, KEK
// Date : 21 - Oct - 2013
//-

#include "daq/dataobjects/RawPXD.h"

using namespace std;
using namespace Belle2;

ClassImp(RawPXD);

RawPXD::RawPXD()
{
  m_nwords = 0;
  m_buffer = NULL;
  m_allocated = false;
}

RawPXD::~RawPXD()
{
  if (m_allocated) delete[] m_buffer;
}

void RawPXD::Copy(int* bufin, int nwords)
{
  //  m_nwords = bufin[0];
  m_nwords = nwords;

  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[m_nwords];

  memcpy((char*)m_buffer, (char*)bufin, m_nwords * sizeof(int));
  m_allocated = true;
}

SendHeader* RawPXD::GetSendHeader()
{
  return &m_header;
}

SendTrailer* RawPXD::GetSendTrailer()
{
  return &m_trailer;
}


int RawPXD::GetBodyNwords()
{
  return m_nwords;
}

int RawPXD::Size()
{
  return m_nwords;
}

int* RawPXD::AllocateBuffer(int nwords)
{
  m_nwords = nwords;
  if (m_allocated) delete[] m_buffer;
  m_buffer = new int[nwords];
  m_allocated = true;
  return m_buffer;
}

int* RawPXD::GetBuffer()
{
  return m_buffer;
}


void RawPXD::SetBuffer(int* bufin, int nwords, int malloc_flag)
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
