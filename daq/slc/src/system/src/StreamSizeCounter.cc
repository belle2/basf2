#include "StreamSizeCounter.hh"

using namespace B2DAQ;

size_t StreamSizeCounter::write(const void*, size_t count)
throw(IOException)
{
  _count += count;
  return count;
}
