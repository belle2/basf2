//+
// File : RawPXD.cc
// Description : RawPXD Data format
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Aug - 2013
//-

#include "rawdata/dataobjects/RawPXD.h"
#include "framework/logging/Logger.h"
using namespace std;
using namespace Belle2;

ClassImp(RawPXD);

RawPXD::RawPXD()
{
  m_nwords = 1;
  m_buffer = new int[1];
  m_allocated = true;
}

RawPXD::RawPXD(int* buffer, int length_in_Bytes)
{
  m_nwords = (length_in_Bytes + 3) / 4;
  m_buffer = buffer;
  m_allocated = false;
}

RawPXD::~RawPXD()
{
  if (m_allocated)
    delete[] m_buffer;
}

int RawPXD::size()
{
  return m_nwords;
}
/*at the moment not used
int* RawPXD::allocate_buffer(int nwords)
{
    m_nwords = nwords;
    m_buffer = new int[nwords];
    m_allocated = true;
    return m_buffer;
}

void RawPXD::data(int nwords, int* data)
{
    memcpy(m_buffer, data, nwords);

    m_buffer[0] = nwords;
}
*/
int* RawPXD::data(void)
{
  int* ptr = &m_buffer[0];
  return ptr;
}
