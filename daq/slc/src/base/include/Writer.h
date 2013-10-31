#ifndef _Belle2_Writer_hh
#define _Belle2_Writer_hh

#include "IOException.h"

namespace Belle2 {

  class Serializable;

  class Writer {

  public:
    virtual ~Writer() throw() {}

  public:
    virtual size_t write(const void*, size_t) throw(IOException) = 0;
    virtual bool available() throw(IOException) = 0;

  protected:
    void reverse(void* buf, size_t len) throw();

  public:
    virtual void writeBool(bool) throw(IOException);
    virtual void writeChar(char) throw(IOException);
    virtual void writeShort(short) throw(IOException);
    virtual void writeInt(int) throw(IOException);
    virtual void writeLong(long long) throw(IOException);
    virtual void writeUChar(unsigned char) throw(IOException);
    virtual void writeUShort(unsigned short) throw(IOException);
    virtual void writeUInt(unsigned int) throw(IOException);
    virtual void writeULong(unsigned long long) throw(IOException);
    virtual void writeFloat(float) throw(IOException);
    virtual void writeDouble(double) throw(IOException);
    virtual void writeString(const std::string&) throw(IOException);
    virtual void writeObject(const Serializable&) throw(IOException);

  };

};

#endif
