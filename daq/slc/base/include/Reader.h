#ifndef _Belle2_Reader_hh
#define _Belle2_Reader_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class Serializable;

  class Reader {

  public:
    virtual ~Reader() {}

  public:
    virtual size_t read(void*, size_t) = 0;
    virtual bool available() = 0;

  protected:
    void reverse(void* buf, size_t len);

  public:
    virtual bool readBool();
    virtual char readChar();
    virtual short readShort();
    virtual int readInt();
    virtual long long readLong();
    virtual unsigned char readUChar();
    virtual unsigned short readUShort();
    virtual unsigned int readUInt();
    virtual unsigned long long readULong();
    virtual float readFloat();
    virtual double readDouble();
    virtual const std::string readString();
    virtual void readObject(Serializable&);

  };

};

#endif
