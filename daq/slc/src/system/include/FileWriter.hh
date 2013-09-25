#ifndef _B2DAQ_FileWriter_hh
#define _B2DAQ_FileWriter_hh

#include "FileDescriptor.hh"

#include <util/Writer.hh>

namespace B2DAQ {
  
  template <class FD>  
  class FileWriter : public Writer {

  public:
    FileWriter() throw() {}
    FileWriter(const FD& fd) throw() : _fd(fd) {}
    virtual ~FileWriter() throw() {}
    
  public:
    virtual size_t write(const void* v, size_t count)
      throw(IOException) {
      return _fd.write(v, count);
    }
    virtual bool available() throw(IOException) {
      return _fd.select();
    }
    
  private:
    FD _fd;

  };

};

#endif
