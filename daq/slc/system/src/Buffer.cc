#include "daq/slc/system/Buffer.h"

#include <cstring>

using namespace Belle2;

Buffer::Buffer()
throw() : m_memory(NULL), m_size(0), m_allocated(false) {}

Buffer::Buffer(unsigned int size, unsigned char* memory) throw()
  : m_memory(memory), m_size(size), m_allocated(false)
{
  if (memory == NULL) {
    m_memory = new unsigned char[size];
    m_allocated = true;
  }
}

Buffer::Buffer(const Buffer& buf) throw()
  : m_memory(NULL), m_size(buf.m_size),
    m_allocated(buf.m_allocated)
{
  if (m_allocated) {
    m_memory = new unsigned char [buf.m_size];
    ::memcpy(m_memory, buf.m_memory, buf.m_size);
  } else {
    m_memory = buf.m_memory;
  }
  m_size = buf.m_size;
}

Buffer::~Buffer() throw()
{
  if (m_allocated && m_memory != NULL) delete [] m_memory;
}

const Buffer& Buffer::operator=(const Buffer& buf) throw()
{
  if (m_allocated) {
    delete [] m_memory;
  }
  m_allocated = buf.m_allocated;
  if (m_allocated) {
    m_memory = new unsigned char [buf.m_size];
    ::memcpy(m_memory, buf.m_memory, buf.m_size);
  } else {
    m_memory = buf.m_memory;
  }
  m_size = buf.m_size;
  return *this;
}
