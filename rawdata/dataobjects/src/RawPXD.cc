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
  m_nwords = 0;
  m_buffer = NULL; // new int[1];
  m_allocated = false;
}

RawPXD::RawPXD(int* buffer, int length_in_Bytes)
{
  m_nwords = (length_in_Bytes + 3) / 4;
  m_buffer = buffer;
  m_allocated = false;
}

unsigned int RawPXD::endian_swap(unsigned int x)
{
  x = (x >> 24) |
      ((x << 8) & 0x00FF0000) |
      ((x >> 8) & 0x0000FF00) |
      (x << 24);
  return x;
}

RawPXD::RawPXD(std::vector <unsigned int>& header, std::vector <std::vector <unsigned char>>& payload)
{

  // Now create header from payload , this can be done with less loops, but speed is not the issue here
  int nr_frames = header.size();
  int payload_size = 0; // in 32 bit words
  for (auto & it : header) {
    payload_size += (it + 3) / 4; // in 32 bit word, rounded up
  }

  m_nwords = 2 + nr_frames + payload_size; // 321 bit words
  m_buffer = new int[m_nwords];
  m_allocated = true;

  B2INFO("RawPXD Frames " << header.size() << " Data " << payload_size << " (32 bitword)");

  // now we know the actual payload length
  int offset = 0;
  m_buffer[offset++] = 0xCAFEBABE;
  m_buffer[offset++] = endian_swap(nr_frames);
  // and now append the frame length table
  for (auto & it : payload) {
    m_buffer[offset++] = endian_swap(it.size()); // in chars, rounded up to 32 bit boundary
  }
  // and now append the actual paylaod data
  unsigned char* data = (unsigned char*) &m_buffer[offset];
  for (auto & it : payload) {
    memcpy(data, it.data(), it.size());
    data += (it.size() + 3) & 0xFFFFFFFC; // in chars, rounded up to 32 bit boundary
  }
  // done
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
