//+
// File : RawPXD.cc
// Description : RawPXD Data format
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Aug - 2013
//-

#include "rawdata/dataobjects/RawPXD.h"
#include "framework/logging/Logger.h"
#include <framework/utilities/HTML.h>

using namespace std;
using namespace Belle2;

RawPXD::RawPXD() : m_nwords(0), m_buffer(NULL)
{
}

RawPXD::RawPXD(int* buffer, int length_in_Bytes)
  : m_nwords(0), m_buffer(NULL)
{
  m_nwords = (length_in_Bytes + 3) / 4;
  m_buffer = new int[m_nwords];
  memcpy(m_buffer, buffer, m_nwords * sizeof(int));
}

unsigned int RawPXD::endian_swap(unsigned int x)
{
  x = (x >> 24) |
      ((x << 8) & 0x00FF0000) |
      ((x >> 8) & 0x0000FF00) |
      (x << 24);
  return x;
}

RawPXD::RawPXD(const std::vector <unsigned int>& header, const std::vector <std::vector <unsigned char>>& payload)
  : m_nwords(0), m_buffer(NULL)
{
  // This function is only used by the PXDPacker in simulations. Does not make sense for other cases
  // Now create header from payload , this can be done with less loops, but speed is not the issue here
  int nr_frames = header.size();
  int payload_size = 0; // in 32 bit words
  for (auto& it : header) {
    payload_size += (it + 3) / 4; // in 32 bit word, rounded up
  }

  m_nwords = 2 + nr_frames + payload_size; // 321 bit words

  m_buffer = new int[m_nwords];

  B2DEBUG(20, "RawPXD Frames " << header.size() << " Data " << payload_size << " (32 bitword)");

  // now we know the actual payload length
  int offset = 0;
  m_buffer[offset++] = 0xBEBAFECA ; // 0xCAFEBABEu;
  m_buffer[offset++] = endian_swap(nr_frames);
  // and now append the frame length table
  for (auto& it : payload) {
    m_buffer[offset++] = endian_swap(it.size()); // in chars, rounded up to 32 bit boundary
  }
  // and now append the actual paylaod data
  unsigned char* data = (unsigned char*) &m_buffer[offset];
  for (auto& it : payload) {
    memcpy(data, it.data(), it.size());
    data += (it.size() + 3) & 0xFFFFFFFC; // in chars, rounded up to 32 bit boundary
  }
  // done
}

RawPXD::~RawPXD()
{
  if (m_buffer != NULL) delete[] m_buffer;
}

int RawPXD::size() const
{
  return m_nwords;
}

int* RawPXD::data(void)
{
  int* ptr = &m_buffer[0];
  return ptr;
}

std::string RawPXD::getInfoHTML() const
{
  std::string s;
  if (m_nwords >= 2)
    s += "Frames: " + std::to_string(endian_swap(m_buffer[1])) + ", ";

  s += "Size (32bit words): " + std::to_string(m_nwords) + "<br /><br />";
  s += HTML::getHexDump(m_buffer, m_nwords);
  return s;
}
