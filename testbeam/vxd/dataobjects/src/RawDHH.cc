//+
// File : RawDHH.cc
// Description : RawDHH Data format
//
// Author : Bjoern Spruck
// Date : 07.04.2016
//-

#include "testbeam/vxd/dataobjects/RawDHH.h"

using namespace std;
using namespace Belle2;

RawDHH::RawDHH() : m_nwords(0), m_buffer(NULL)
{
}

RawDHH::RawDHH(int* buffer, int length_in_Bytes)
  : m_nwords(0), m_buffer(NULL)
{
  m_nwords = (length_in_Bytes + 3) / 4;
  m_buffer = new int[m_nwords];
  memcpy(m_buffer, buffer, m_nwords * sizeof(int));
}

RawDHH::~RawDHH()
{
  if (m_buffer != NULL) delete[] m_buffer;
}

int RawDHH::size()
{
  return m_nwords;
}

int* RawDHH::data(void)
{
  int* ptr = &m_buffer[0];
  return ptr;
}
