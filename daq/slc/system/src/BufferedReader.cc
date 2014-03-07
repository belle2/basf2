#include "daq/slc/system/BufferedReader.h"

#include <cstring>

using namespace Belle2;

BufferedReader::BufferedReader() throw()
  : _memory(NULL), _size(0), _pos(0), _allocated(false) {}

BufferedReader::BufferedReader(const BufferedReader& reader) throw()
  : _memory(NULL), _size(reader._size),
    _pos(reader._pos), _allocated(reader._allocated)
{
  if (_allocated) {
    _memory = new unsigned char [reader._size];
    ::memcpy(_memory, reader._memory, reader._size);
  } else {
    _memory = reader._memory;
  }
  _size = reader._size;
  _pos = reader._pos;
}

BufferedReader::BufferedReader(size_t size, unsigned char* memory) throw()
  : _memory(memory), _size(size), _pos(0), _allocated(false)
{
  if (memory == NULL) {
    _memory = new unsigned char[size];
    _allocated = true;
  }
}

BufferedReader::~BufferedReader() throw()
{
  if (_allocated && _memory != NULL) {
    delete [] _memory;
  }
}

const BufferedReader& BufferedReader::operator=(const BufferedReader& reader) throw()
{
  if (_allocated) {
    delete [] _memory;
  }
  _allocated = reader._allocated;
  if (_allocated) {
    _memory = new unsigned char [reader._size];
    ::memcpy(_memory, reader._memory, reader._size);
  } else {
    _memory = reader._memory;
  }
  _size = reader._size;
  _pos = reader._pos;
  return *this;
}

void BufferedReader::copy(const void* buffer, size_t count) throw(IOException)
{
  if (count < _size) {
    memcpy(_memory, buffer, count);
    _pos = 0;
  } else {
    throw (IOException(__FILE__, __LINE__, "Buffer over flow"));
  }
}

size_t BufferedReader::read(void* buf, const size_t count) throw(IOException)
{
  if (_pos + count > size()) {
    throw (IOException(__FILE__, __LINE__, "Buffer over flow"));
  }
  memcpy(buf, (_memory + _pos), count);
  _pos += count;
  return count;
}

