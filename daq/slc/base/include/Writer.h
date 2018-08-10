#ifndef _Belle2_Writer_hh
#define _Belle2_Writer_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class Serializable;

  class Writer {

  public:
    virtual ~Writer() {}

  public:
    virtual size_t write(const void*, size_t) = 0;
    virtual bool available() = 0;

  protected:
    void reverse(void* buf, size_t len);

  public:
    virtual void writeBool(bool);
    virtual void writeChar(char);
    virtual void writeShort(short);
    virtual void writeInt(int);
    virtual void writeLong(long long);
    virtual void writeUChar(unsigned char);
    virtual void writeUShort(unsigned short);
    virtual void writeUInt(unsigned int);
    virtual void writeULong(unsigned long long);
    virtual void writeFloat(float);
    virtual void writeDouble(double);
    virtual void writeString(const std::string&);
    virtual void writeObject(const Serializable&);

  };

};

#endif
