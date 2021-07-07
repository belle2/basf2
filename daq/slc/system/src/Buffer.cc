/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Buffer.h"

#include <cstring>

using namespace Belle2;

Buffer::Buffer() : m_memory(NULL), m_size(0), m_allocated(false) {}

Buffer::Buffer(unsigned int size, unsigned char* memory)
  : m_memory(memory), m_size(size), m_allocated(false)
{
  if (memory == NULL && size > 0) {
    m_memory = new unsigned char[size];
    ::memset(m_memory, 0, size);
    m_allocated = true;
  }
}

Buffer::Buffer(const Buffer& buf)
  : m_memory(NULL), m_size(buf.m_size),
    m_allocated(buf.m_allocated)
{
  if (m_allocated) {
    m_memory = new unsigned char [buf.m_size];
    ::memset(m_memory, 0, buf.m_size);
    ::memcpy(m_memory, buf.m_memory, buf.m_size);
  } else {
    m_memory = buf.m_memory;
  }
  m_size = buf.m_size;
}

Buffer::~Buffer()
{
  if (m_allocated && m_memory != NULL) {
    delete [] m_memory;
    m_allocated = false;
    m_memory = NULL;
  }
}

const Buffer& Buffer::operator=(const Buffer& buf)
{
  if (m_allocated) {
    delete [] m_memory;
  }
  m_allocated = buf.m_allocated;
  if (m_allocated) {
    m_memory = new unsigned char [buf.m_size];
    ::memset(m_memory, 0, buf.m_size);
    ::memcpy(m_memory, buf.m_memory, buf.m_size);
  } else {
    m_memory = buf.m_memory;
  }
  m_size = buf.m_size;
  return *this;
}
