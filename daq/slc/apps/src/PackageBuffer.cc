#include "daq/slc/apps/PackageBuffer.h"

#include <cstdlib>
#include <cstring>

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
  StreamSizeCounter counter;
  counter.reset();
  _lock.wrlock();
  serializeAll(counter);
  //size_t count = (size_t)(counter.count() * 1.0);
  if (_writer != NULL) delete _writer;
  //_writer = new ZipDeflater(counter.count(), count);
  _writer = new BufferedWriter(counter.count());
  _lock.unlock();
  return counter.count();
}

void PackageBuffer::update()
{
  _lock.wrlock();
  _writer->seekTo(0);
  serialize(*_writer);
  //_writer->deflate();
  _lock.unlock();
}

size_t PackageBuffer::size()
{
  _lock.rdlock();
  //size_t size =  _writer->getBufferSize();
  size_t size =  _writer->size();
  _lock.unlock();
  return size;
}

char* PackageBuffer::createBuffer(size_t& buf_size)
{
  _lock.rdlock();
  //buf_size = _writer->getBufferSize();
  buf_size =  _writer->size();
  _lock.unlock();
  return (char*)::malloc(buf_size);
}

size_t PackageBuffer::copy(char* buf, size_t buf_size)
{
  _lock.rdlock();
  //size_t size =  _writer->getCompSize();
  size_t size =  _writer->size();
  if (buf_size >= size) {
    //memcpy(buf, _writer->getCompBuffer(), size);
    memcpy(buf, _writer->ptr(), size);
  } else {
    size = 0;
  }
  _lock.unlock();
  return size;
}
