#include "PackageBuffer.h"

#include <base/Debugger.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Belle2;

PackageBuffer::PackageBuffer()
{
  _pack = NULL;
  _writer = NULL;
}

PackageBuffer::~PackageBuffer() throw()
{
  if (_writer != NULL) delete _writer;
}

size_t PackageBuffer::allocate(HistoPackage* pack)
{
  _pack = pack;
  _lock.init();
  Belle2::StreamSizeCounter counter;
  counter.reset();
  _lock.wrlock();
  serializeAll(counter);
  size_t count = (size_t)(counter.count() * 1.2);
  if (_writer != NULL) delete _writer;
  _writer = new Belle2::ZipDeflater(counter.count(), count);
  _lock.unlock();
  return count;
}

void PackageBuffer::update()
{
  _lock.wrlock();
  _writer->seekTo(0);
  serialize(*_writer);
  _writer->deflate();
  _lock.unlock();
}

size_t PackageBuffer::size()
{
  _lock.rdlock();
  size_t size =  _writer->getBufferSize();
  _lock.unlock();
  return size;
}

char* PackageBuffer::createBuffer(size_t& buf_size)
{
  _lock.rdlock();
  buf_size = _writer->getBufferSize();
  _lock.unlock();
  return (char*)::malloc(buf_size);
}

size_t PackageBuffer::copy(char* buf, size_t buf_size)
{
  _lock.rdlock();
  size_t size =  _writer->getCompSize();
  if (buf_size >= size) {
    memcpy(buf, _writer->getCompBuffer(), size);
  } else {
    size = 0;
  }
  _lock.unlock();
  return size;
}
