#include "daq/slc/system/BufferedWriter.h"

#include <cstring>

using namespace Belle2;

BufferedWriter::BufferedWriter()
throw() : _memory(NULL), _size(0), _pos(0), _allocated(false) {}

BufferedWriter::BufferedWriter(size_t size, unsigned char* memory) throw()
  : _memory(memory), _size(size), _pos(0), _allocated(false)
{
  if (memory == NULL) {
    _memory = new unsigned char[size];
    _allocated = true;
  }
}

BufferedWriter::BufferedWriter(const BufferedWriter& writer) throw()
  : _memory(NULL), _size(writer._size),
    _pos(writer._pos), _allocated(writer._allocated)
{
  if (_allocated) {
    _memory = new unsigned char [writer._size];
    ::memcpy(_memory, writer._memory, writer._size);
  } else {
    _memory = writer._memory;
  }
  _size = writer._size;
  _pos = writer._pos;
}

BufferedWriter::~BufferedWriter() throw()
{
  if (_allocated && _memory != NULL) delete [] _memory;
}

const BufferedWriter& BufferedWriter::operator=(const BufferedWriter& writer) throw()
{
  if (_allocated) {
    delete [] _memory;
  }
  _allocated = writer._allocated;
  if (_allocated) {
    _memory = new unsigned char [writer._size];
    ::memcpy(_memory, writer._memory, writer._size);
  } else {
    _memory = writer._memory;
  }
  _size = writer._size;
  _pos = writer._pos;
  return *this;
}

size_t BufferedWriter::write(const void* buf, const size_t count) throw(IOException)
{
  if (_pos + count > _size) {
    throw (IOException(__FILE__, __LINE__, "over run"));
  }
  memcpy((_memory + _pos), buf, count);
  _pos += count;
  return count;
}

