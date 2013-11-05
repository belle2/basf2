#include "system/BufferedWriter.h"

#include <cstring>

using namespace Belle2;

BufferedWriter::BufferedWriter()
throw() : _memory(NULL), _size(0), _pos(0)  {}

BufferedWriter::BufferedWriter(size_t size) throw()
  : _memory(new unsigned char[size]), _size(size), _pos(0) {}

BufferedWriter::BufferedWriter(const BufferedWriter& writer) throw()
  : _memory(NULL), _size(writer._size), _pos(writer._pos)
{
  _memory = new unsigned char [writer._size];
  _size = writer._size;
  _pos = writer._pos;
  ::memcpy(_memory, writer._memory, _size);
}

BufferedWriter::~BufferedWriter() throw() {}

const BufferedWriter& BufferedWriter::operator=(const BufferedWriter& writer) throw()
{
  if (_memory != NULL) {
    delete [] _memory;
  }
  _memory = new unsigned char [writer._size];
  _size = writer._size;
  _pos = writer._pos;
  ::memcpy(_memory, writer._memory, _size);
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

