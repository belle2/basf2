//+
// File : rawheader.cc
// Description : pseudo raw data header for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#include "daq/rawdata/RawHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(RawHeader);

RawHeader::RawHeader()
{
  cout << "RawHeader NULL constructor" << endl;
  m_buffer = NULL;
  m_nwords = 0;
  m_allocated = false;
}

RawHeader::RawHeader(int nwords, unsigned int* buffer)
{
  cout << "RawHeader constructor with buffer" << endl;
  m_nwords = nwords;
  m_buffer = buffer;
  m_allocated = false;
}

RawHeader::~RawHeader()
{
  if (m_allocated) delete[] m_buffer;
}

void RawHeader::copy(int nwords, unsigned int* bufin)
{
  m_buffer = new unsigned int[nwords];
  memcpy((char*)m_buffer, (char*)bufin, nwords * sizeof(unsigned int));
  m_allocated = true;
}

unsigned int* RawHeader::allocate_buffer(int nwords)
{
  m_buffer = new unsigned int[nwords];
  m_allocated = true;
  return m_buffer;
}

unsigned int* RawHeader::get_buffer()
{
  return m_buffer;
}

void RawHeader::set_buffer(int nwords, unsigned int* bufin)
{
  printf("&m_buffer = %8.8x, m_buffer = %8.8x\n", &m_buffer, m_buffer);
  m_buffer = bufin;
  m_nwords = nwords;
}
