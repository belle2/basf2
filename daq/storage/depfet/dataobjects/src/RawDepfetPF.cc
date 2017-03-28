//+
// File : RawDepfetPF.cc
// Description : RawDepfetPF Data format
//
// Author : Tomoyuki Konno, KEK
// Date : 13 - Aug - 2013
//-

#include "daq/storage/depfet/dataobjects/RawDepfetPF.h"
#include "framework/logging/Logger.h"

using namespace std;
using namespace Belle2;

ClassImp(RawDepfetPF);

RawDepfetPF::RawDepfetPF() : m_nwords(0), m_buffer(NULL)
{
}

RawDepfetPF::RawDepfetPF(int* buffer, int length_in_Bytes)
  : m_nwords(0), m_buffer(NULL)
{
  m_nwords = (length_in_Bytes + 3) / 4;
  m_buffer = new int[m_nwords];
  memcpy(m_buffer, buffer, m_nwords * sizeof(int));
}

unsigned int RawDepfetPF::endian_swap(unsigned int x)
{
  x = (x >> 24) |
      ((x << 8) & 0x00FF0000) |
      ((x >> 8) & 0x0000FF00) |
      (x << 24);
  return x;
}

RawDepfetPF::~RawDepfetPF()
{
  if (m_buffer != NULL) delete[] m_buffer;
}

int RawDepfetPF::size()
{
  return m_nwords;
}

int* RawDepfetPF::data(void)
{
  int* ptr = &m_buffer[0];
  return ptr;
}
