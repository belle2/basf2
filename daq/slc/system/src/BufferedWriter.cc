#include "daq/slc/system/BufferedWriter.h"

#include <cstring>

using namespace Belle2;

BufferedWriter::BufferedWriter()
throw() : m_memory(NULL), m_size(0), m_pos(0), m_allocated(false) {}

BufferedWriter::BufferedWriter(size_t size, unsigned char* memory) throw()
  : m_memory(memory), m_size(size), m_pos(0), m_allocated(false)
{
  if (memory == NULL) {
    m_memory = new unsigned char[size];
    m_allocated = true;
  }
}

BufferedWriter::BufferedWriter(const BufferedWriter& writer) throw()
  : m_memory(NULL), m_size(writer.m_size),
    m_pos(writer.m_pos), m_allocated(writer.m_allocated)
{
  if (m_allocated) {
    m_memory = new unsigned char [writer.m_size];
    ::memcpy(m_memory, writer.m_memory, writer.m_size);
  } else {
    m_memory = writer.m_memory;
  }
  m_size = writer.m_size;
  m_pos = writer.m_pos;
}

BufferedWriter::~BufferedWriter() throw()
{
  if (m_allocated && m_memory != NULL) delete [] m_memory;
}

const BufferedWriter& BufferedWriter::operator=(const BufferedWriter& writer) throw()
{
  if (m_allocated) {
    delete [] m_memory;
  }
  m_allocated = writer.m_allocated;
  if (m_allocated) {
    m_memory = new unsigned char [writer.m_size];
    ::memcpy(m_memory, writer.m_memory, writer.m_size);
  } else {
    m_memory = writer.m_memory;
  }
  m_size = writer.m_size;
  m_pos = writer.m_pos;
  return *this;
}

size_t BufferedWriter::write(const void* buf, const size_t count) throw(IOException)
{
  if (m_pos + count > m_size) {
    throw (IOException("out of buffer range: %d+%d>%d",
                       (int)m_pos, (int)count, (int)m_size));
  }
  memcpy((m_memory + m_pos), buf, count);
  m_pos += count;
  return count;
}

