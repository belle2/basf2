#include "daq/storage/SharedEventBuffer.h"

#include <daq/slc/system/LogFile.h>

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <iostream>

using namespace Belle2;

size_t SharedEventBuffer::size() throw()
{
  return m_mutex.size() + m_cond.size() +
         sizeof(Header) + sizeof(int) * (m_nword);
}

bool SharedEventBuffer::open(const std::string& nodename,
                             size_t nword, bool recreate)
{
  m_nword = nword;
  std::string username = getenv("USER");
  m_path = "/storage_info_" + username + "_" + nodename;
  LogFile::debug("%s", m_path.c_str());
  //if (recreate) SharedMemory::unlink(m_path);
  if (!m_memory.open(m_path, size())) {
    perror("shm_open");
    LogFile::fatal("Failed to open %s", m_path.c_str());
    return false;
  }
  char* buf = (char*) m_memory.map(0, size());
  if (buf == NULL) {
    return false;
  }
  m_mutex = MMutex(buf);
  buf += m_mutex.size();
  m_cond = MCond(buf);
  buf += m_cond.size();
  m_header = (Header*)buf;
  buf += sizeof(Header);
  m_buf = (int*)buf;
  if (recreate) init();
  return true;
}

bool SharedEventBuffer::init()
{
  if (m_buf == NULL) return false;
  m_mutex.init();
  m_cond.init();
  memset(m_header, 0, sizeof(Header));
  memset(m_buf, 0, sizeof(int) * m_nword);
  //for (unsigned long long i = 0; i < m_nword; i++) {
  //  m_buf[i] = 0;
  //}
  return true;
}

void SharedEventBuffer::clear()
{
  if (m_buf == NULL) return;
  m_mutex.lock();
  memset(m_header, 0, sizeof(Header));
  for (unsigned long long i = 0; i < m_nword; i++) {
    m_buf[i] = 0;
  }
  m_mutex.unlock();
}

bool SharedEventBuffer::close()
{
  m_memory.close();
  return true;
}

bool SharedEventBuffer::unlink()
{
  m_memory.close();
  m_memory.unlink();
  return true;
}

bool SharedEventBuffer::lock() throw()
{
  if (m_buf == NULL) return false;
  return m_mutex.lock();
}

bool SharedEventBuffer::unlock() throw()
{
  if (m_buf == NULL) return false;
  return m_mutex.unlock();
}

bool SharedEventBuffer::wait() throw()
{
  if (m_buf == NULL) return false;
  return m_cond.wait(m_mutex);
}

bool SharedEventBuffer::wait(int time) throw()
{
  if (m_buf == NULL) return false;
  return m_cond.wait(m_mutex, time, 0);
}

bool SharedEventBuffer::notify() throw()
{
  if (m_buf == NULL) return false;
  return m_cond.broadcast();
}

bool SharedEventBuffer::isWritable(int nword) throw()
{
  if (m_buf == NULL) return false;
  m_mutex.lock();
  bool writable = m_header->nword_in - m_header->nword_out < m_nword - (nword + 1);
  m_mutex.unlock();
  return writable;
}

bool SharedEventBuffer::isReadable(int nword) throw()
{
  if (m_buf == NULL) return false;
  m_mutex.lock();
  bool readable = m_header->nword_in - m_header->nword_out >= m_nword - (nword + 1);
  m_mutex.unlock();
  return readable;

}

unsigned int SharedEventBuffer::write(const int* buf, unsigned int nword,
                                      bool fouce, unsigned int serial, bool unlocked)
{
  if (m_buf == NULL) return 0;
  if (nword == 0) return 0;
  if (nword > m_nword) return -1;
  if (!unlocked) m_mutex.lock();
  m_header->nwriter++;
  while (!fouce && m_header->nreader > 0) {
    m_cond.wait(m_mutex);
  }
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  while (true) {
    i_w = m_header->nword_in % m_nword;
    i_r = m_header->nword_out % m_nword;
    if ((serial == 0 || serial - 1 == m_header->count_in) &&
        m_header->nword_in - m_header->nword_out < m_nword - (nword + 1)) {
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
    m_header->nwriter--;
    m_cond.wait(m_mutex);
    m_header->nwriter++;
  }
  m_header->nword_in += nword + 1;
  unsigned int count = ++m_header->count_in;
  m_header->nwriter--;
  m_cond.broadcast();
  if (!unlocked) m_mutex.unlock();
  return count;
}

unsigned int SharedEventBuffer::read(int* buf, bool fouce, bool unlocked,
                                     SharedEventBuffer::Header* hdr)
{
  if (m_buf == NULL) return 0;
  if (!unlocked) m_mutex.lock();
  m_header->nreader++;
  while (!fouce && m_header->nwriter > 0) {
    m_cond.wait(m_mutex);
  }
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  unsigned int nword = 0;
  while (true) {
    i_w = m_header->nword_in % m_nword;
    i_r = m_header->nword_out % m_nword;
    nword = m_buf[i_r];
    if (nword > 0) {
      if (m_header->nword_in - m_header->nword_out >= (nword + 1)) {
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
    m_header->nreader--;
    m_cond.wait(m_mutex);
    m_header->nreader++;
  }
  m_header->nword_out += nword + 1;
  unsigned int count = ++m_header->count_out;
  m_header->nreader--;
  if (hdr != NULL) {
    memcpy(hdr, m_header, sizeof(SharedEventBuffer::Header));
  }
  m_cond.broadcast();
  if (!unlocked) m_mutex.unlock();
  return count;
}
