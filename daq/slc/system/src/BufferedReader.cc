#include "daq/slc/system/BufferedReader.h"

#include <cstring>

using namespace Belle2;

BufferedReader::BufferedReader() throw()
  : _memory(NULL), _size(0), _pos(0) {}

BufferedReader::BufferedReader(const BufferedReader& reader) throw()
  : _memory(NULL), _size(reader._size), _pos(reader._pos)
{
  _memory = new unsigned char [reader._size];
  _size = reader._size;
  _pos = reader._pos;
  memcpy(_memory, reader._memory, _size);
}

BufferedReader::BufferedReader(size_t size) throw()
  : _memory(new unsigned char[size]), _size(size), _pos(0) {}

BufferedReader::~BufferedReader() throw() {}

const BufferedReader& BufferedReader::operator=(const BufferedReader& reader) throw()
{
  if (_memory != NULL) {
    delete [] _memory;
  }
  _memory = new unsigned char [reader._size];
  _size = reader._size;
  _pos = reader._pos;
  memcpy(_memory, reader._memory, _size);
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

