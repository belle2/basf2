/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/storage/EventBuffer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

unsigned int EventBuffer::size() throw()
{
  return sizeof(int) * (m_nword);
}

EventBuffer::EventBuffer(unsigned int nword)
{
  m_nword = nword;
  char* buf = (char*) malloc(size());
  if (buf == NULL) {
    return;
  }
  m_buf = (int*)buf;
  memset(&m_header, 0, sizeof(Header));
  for (unsigned long long i = 0; i < m_nword; i++) {
    m_buf[i] = 0;
  }
}

void EventBuffer::clear()
{
  if (m_buf == NULL) return;
  memset(&m_header, 0, sizeof(Header));
  for (unsigned long long i = 0; i < m_nword; i++) {
    m_buf[i] = 0;
  }
}

EventBuffer::~EventBuffer()
{
  free(m_buf);
}

bool EventBuffer::isWritable(int nword) throw()
{
  if (m_buf == NULL) return false;
  bool writable = m_header.nword_in - m_header.nword_out < m_nword - (nword + 1);
  return writable;
}

bool EventBuffer::isReadable() throw()
{
  if (m_buf == NULL) return false;
  bool readable = m_header.nword_in - m_header.nword_out > 0;
  return readable;
}

unsigned int EventBuffer::write(const int* buf, unsigned int nword,
                                unsigned int serial)
{
  if (m_buf == NULL) return 0;
  if (nword == 0) return 0;
  if (nword > m_nword) return -1;
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  while (true) {
    i_w = m_header.nword_in % m_nword;
    i_r = m_header.nword_out % m_nword;
    if ((serial == 0 || serial - 1 == m_header.count_in) &&
        m_header.nword_in - m_header.nword_out < m_nword - (nword + 1)) {
      if (i_w >= i_r) {
        unsigned int count = m_nword - i_w;
        if (nword + 1 < count) {
          m_buf[i_w] = nword;
          memcpy((m_buf + i_w + 1), buf, sizeof(int) * nword);
        } else {
          m_buf[i_w] = nword;
          memcpy((m_buf + i_w + 1), buf, sizeof(int) * count);
          if (nword >= count)
            memcpy(m_buf, buf + count, sizeof(int) * (nword - count));
        }
      } else {
        m_buf[i_w] = nword;
        memcpy((m_buf + i_w + 1), buf, sizeof(int) * nword);
      }
      break;
    }
  }
  m_header.nword_in += nword + 1;
  unsigned int count = ++m_header.count_in;
  return count;
}

unsigned int EventBuffer::read(int* buf, EventBuffer::Header* hdr)
{
  if (m_buf == NULL) return 0;
  m_header.nreader++;
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  unsigned int nword = 0;
  while (true) {
    i_w = m_header.nword_in % m_nword;
    i_r = m_header.nword_out % m_nword;
    nword = m_buf[i_r];
    if (nword > 0) {
      if (m_header.nword_in - m_header.nword_out >= (nword + 1)) {
        if (i_w > i_r) {
          memcpy(buf, (m_buf + i_r + 1), sizeof(int) * nword);
          break;
        } else if (i_w < i_r) {
          if (m_nword - i_r > nword) {
            memcpy(buf, (m_buf + i_r + 1), sizeof(int) * nword);
            break;
          } else {
            unsigned int count = m_nword - i_r;
            memcpy(buf, (m_buf + i_r + 1), sizeof(int) * count);
            if (nword > count) {
              memcpy(buf + count, m_buf, sizeof(int) * (nword - count));
            }
            break;
          }
        }
      }
    }
  }
  m_header.nword_out += nword + 1;
  unsigned int count = ++m_header.count_out;
  m_header.nreader--;
  if (hdr != NULL) {
    memcpy(hdr, &m_header, sizeof(EventBuffer::Header));
  }
  return count;
}
