#include "ZipDeflater.h"

#include <stdio.h>
#include <zlib.h>

using namespace Belle2;

ZipDeflater::ZipDeflater() throw():
  BufferedWriter(1024), _comp_buf(new unsigned char[1200]),
  _comp_buf_size(1200), _comp_size(0)
{
}

ZipDeflater::ZipDeflater(size_t size, size_t comp_buf_size)
throw() :  BufferedWriter(size),
  _comp_buf(new unsigned char[comp_buf_size]),
  _comp_buf_size(comp_buf_size), _comp_size(0)
{
}

ZipDeflater::~ZipDeflater() throw()
{
  delete [] _comp_buf;
}

void ZipDeflater::deflate(int level) throw(IOException)
{
  _comp_size = _comp_buf_size;
  int e = 0;
  if ((e =::compress2(_comp_buf, &_comp_size, ptr(), count(), level))
      != Z_OK) {
    printf("_comp_buf_size=%d\n", (int)_comp_buf_size);
    switch (e) {
      case Z_BUF_ERROR: printf("Buffer error\n"); break;
      case Z_MEM_ERROR: printf("Memory error\n"); break;
      case Z_STREAM_ERROR: printf("Stream error\n"); break;
      default: printf("Unknown error\n"); break;
    }
    throw (IOException(__FILE__, __LINE__, "Failed to deflate"));
  }
}

void ZipDeflater::writeObject(Writer& writer) throw(IOException)
{
  writer.writeInt(_comp_size);
  writer.write(_comp_buf, _comp_size);
  seekTo(0);
}
