#include "daq/slc/system/BufferedReader.h"

#include <cstring>

using namespace Belle2;

BufferedReader::BufferedReader() throw()
  : m_memory(NULL), m_size(0), m_pos(0), m_allocated(false) {}

BufferedReader::BufferedReader(const BufferedReader& reader) throw()
  : m_memory(NULL), m_size(reader.m_size),
    m_pos(reader.m_pos), m_allocated(reader.m_allocated)
{
  if (m_allocated) {
    m_memory = new unsigned char [reader.m_size];
    ::memcpy(m_memory, reader.m_memory, reader.m_size);
  } else {
    m_memory = reader.m_memory;
  }
  m_size = reader.m_size;
  m_pos = reader.m_pos;
}

BufferedReader::BufferedReader(size_t size, unsigned char* memory) throw()
  : m_memory(memory), m_size(size), m_pos(0), m_allocated(false)
{
  if (memory == NULL) {
    m_memory = new unsigned char[size];
    m_allocated = true;
  }
}

BufferedReader::~BufferedReader() throw()
{
  if (m_allocated && m_memory != NULL) {
    delete [] m_memory;
  }
}

const BufferedReader& BufferedReader::operator=(const BufferedReader& reader) throw()
{
  if (m_allocated) {
    delete [] m_memory;
  }
  m_allocated = reader.m_allocated;
  if (m_allocated) {
    m_memory = new unsigned char [reader.m_size];
    ::memcpy(m_memory, reader.m_memory, reader.m_size);
  } else {
    m_memory = reader.m_memory;
  }
  m_size = reader.m_size;
  m_pos = reader.m_pos;
  return *this;
}

void BufferedReader::copy(const void* buffer, size_t count) throw(IOException)
{
  if (count < m_size) {
    memcpy(m_memory, buffer, count);
    m_pos = 0;
  } else {
    throw (IOException("Out of buffer range: %d > %d",
                       (int)count, (int)m_size));
  }
}

size_t BufferedReader::read(void* buf, const size_t count) throw(IOException)
{
  if (m_pos + count > size()) {
    throw (IOException("Out of buffer range: %d + %d > %d",
                       (int)m_pos, (int)count, (int)m_size));
  }
  memcpy(buf, (m_memory + m_pos), count);
  m_pos += count;
  return count;
}

