//+
// File : rawcdc.cc
// Description : pseudo raw data of CDC for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#include "daq/rawdata/RawCDC.h"

using namespace std;
using namespace Belle2;

ClassImp(RawCDC);

RawCDC::RawCDC()
{
  m_allocated = false;
}

RawCDC::RawCDC(int nwords, unsigned int* buffer)
{
  m_nwords = nwords;
  m_buffer = buffer;
  m_allocated = false;
}

RawCDC::~RawCDC()
{
  if (m_allocated) delete[] m_buffer;
}

void RawCDC::copy(int nwords, unsigned int* bufin)
{
  m_buffer = new unsigned int[nwords];
  memcpy((char*)m_buffer, (char*)bufin, nwords*sizeof(unsigned int));
  m_allocated = true;
}

unsigned int* RawCDC::allocate_buffer(int nwords)
{
  m_buffer = new unsigned int[nwords];
  m_allocated = true;
  return m_buffer;
}

unsigned int* RawCDC::get_buffer()
{
  return m_buffer;
}

void RawCDC::set_buffer(int nwords, unsigned int* bufin)
{
  m_buffer = bufin;
  m_nwords = nwords;
}
