#ifndef _B2DAQ_BufferedWriter_hh
#define _B2DAQ_BufferedWriter_hh

#include <util/Writer.hh>

namespace B2DAQ { 

  class BufferedWriter : public Writer {
 
    // member data
  protected:
    unsigned char* _memory;
    size_t _size;
    size_t _pos;

    // constructors & destructors 
  public:
    BufferedWriter() throw();
    BufferedWriter(size_t size) throw();
    BufferedWriter(const BufferedWriter& w) throw();
    virtual ~BufferedWriter() throw();

    // member methods implemented
  public: 
    void seekTo(size_t pos) throw() { _pos = pos; }
    unsigned char* ptr() throw() { return _memory; }
    size_t count() const throw() { return _pos; }
    size_t size() const throw() { return _size; }
    virtual size_t write(const void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }

    // operators
  public:
    const BufferedWriter& operator = (const BufferedWriter&) throw();

  };
 
}
 
#endif
