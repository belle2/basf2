#ifndef _Belle2_Reader_hh
#define _Belle2_Reader_hh

#include "base/IOException.h"

namespace Belle2 {

  class Serializable;

  class Reader {

  public:
    virtual ~Reader() throw() {}

  public:
    virtual size_t read(void*, size_t) throw(IOException) = 0;
    virtual bool available() throw(IOException) = 0;

  protected:
    void reverse(void* buf, size_t len) throw();

  public:
    virtual bool readBool() throw(IOException);
    virtual char readChar() throw(IOException);
    virtual short readShort() throw(IOException);
    virtual int readInt() throw(IOException);
    virtual long long readLong() throw(IOException);
    virtual unsigned char readUChar() throw(IOException);
    virtual unsigned short readUShort() throw(IOException);
    virtual unsigned int readUInt() throw(IOException);
    virtual unsigned long long readULong() throw(IOException);
    virtual float readFloat() throw(IOException);
    virtual double readDouble() throw(IOException);
    virtual const std::string readString() throw(IOException);
    virtual void readObject(Serializable&) throw(IOException);

  };

};

#endif
