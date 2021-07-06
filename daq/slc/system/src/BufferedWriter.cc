/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/BufferedWriter.h"

#include <daq/slc/base/IOException.h>

#include <cstring>

using namespace Belle2;

BufferedWriter::BufferedWriter()
  : m_memory(NULL), m_size(0), m_pos(0), m_allocated(false) {}

BufferedWriter::BufferedWriter(size_t size, unsigned char* memory)
  : m_memory(memory), m_size(size), m_pos(0), m_allocated(false)
{
  if (memory == NULL) {
    m_memory = new unsigned char[size];
    m_allocated = true;
  }
}

BufferedWriter::BufferedWriter(const BufferedWriter& writer)
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

BufferedWriter::~BufferedWriter()
{
  if (m_allocated && m_memory != NULL) delete [] m_memory;
}

const BufferedWriter& BufferedWriter::operator=(const BufferedWriter& writer)
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

size_t BufferedWriter::write(const void* buf, const size_t count)
{
  if (m_pos + count > m_size) {
    throw (IOException("out of buffer range: %d+%d>%d",
                       (int)m_pos, (int)count, (int)m_size));
  }
  memcpy((m_memory + m_pos), buf, count);
  m_pos += count;
  return count;
}

