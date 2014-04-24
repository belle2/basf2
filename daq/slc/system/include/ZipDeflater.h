#ifndef _Belle2_ZipDeflater_hh
#define _Belle2_ZipDeflater_hh

#include "daq/slc/system/BufferedWriter.h"

#include <string>
#include <zlib.h>

namespace Belle2 {

  class ZipDeflater : public BufferedWriter {

    // constructors & destructors
  public:
    ZipDeflater() throw();
    ZipDeflater(size_t size, size_t comp_buf_size) throw();
    virtual ~ZipDeflater() throw();

    // member methods implemented
  public:
    void* getCompBuffer() const throw() { return _comp_buf; }
    size_t getBufferSize() const throw() { return _comp_buf_size; }
    size_t getCompSize() const throw() { return _comp_size; }
    void deflate(int level = 1) throw(IOException);
    void writeObject(Writer&) throw(IOException);

    // member data
  private:
    unsigned char* _comp_buf;
    uLongf _comp_buf_size;
    uLongf _comp_size;

  };

}

#endif
