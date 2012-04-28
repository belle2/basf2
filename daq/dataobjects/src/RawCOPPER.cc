//+
// File : RawCOPPER.cc
// Description : COPPER Raw Data Handler
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 15 - Dec - 2011
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

RawCOPPER::RawCOPPER(int* buffer)
{
  m_nwords = buffer[0];
  m_buffer = buffer;
  m_allocated = false;
}

RawCOPPER::~RawCOPPER()
{
  if (m_allocated) delete[] m_buffer;
}

void RawCOPPER::copy(int* bufin)
{
  m_nwords = bufin[0];
  m_buffer = new int[m_nwords];
  memcpy((char*)m_buffer, (char*)bufin, m_nwords * sizeof(int));
  m_allocated = true;
}

int* RawCOPPER::allocate_buffer(int nwords)
{
  m_nwords = nwords;
  m_buffer = new int[nwords];
  m_allocated = true;
  return m_buffer;
}

int* RawCOPPER::buffer()
{
  return m_buffer;
}

void RawCOPPER::buffer(int* bufin)
{
  m_nwords = bufin[0];
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


