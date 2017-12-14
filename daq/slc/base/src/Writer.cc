#include "daq/slc/base/Writer.h"

#include "daq/slc/base/Serializable.h"

#define __ENDIAN_L__

using namespace Belle2;

void Writer::reverse(void* buf, size_t len) throw()
{
  char tmp;
  for (int c = 0 ; c < (int)len / 2 ; c ++) {
    tmp = *((char*)buf + c);
    *((char*)buf + c) = *((char*)buf + len - 1 - c);
    *((char*)buf + len - 1 - c) = tmp;
  }
}

void Writer::writeBool(bool v) throw(IOException)
{
  write(&v, sizeof(bool));
}

void Writer::writeChar(char v) throw(IOException)
{
  write(&v, sizeof(char));
}

void Writer::writeUChar(unsigned char v) throw(IOException)
{
  write(&v, sizeof(unsigned char));
}

void Writer::writeShort(short v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(short));
#endif
  write(&v, sizeof(short));
}

void Writer::writeUShort(unsigned short v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned short));
#endif
  write(&v, sizeof(unsigned short));
}

void Writer::writeInt(int v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(int));
#endif
  write(&v, sizeof(int));
}

void Writer::writeUInt(unsigned int v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned int));
#endif
  write(&v, sizeof(unsigned int));
}

void Writer::writeLong(long long v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(long long));
#endif
  write(&v, sizeof(long long));
}

void Writer::writeULong(unsigned long long v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned long long));
#endif
  write(&v, sizeof(unsigned long long));
}

void Writer::writeFloat(float v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(float));
#endif
  write(&v, sizeof(float));
}

void Writer::writeDouble(double v) throw(IOException)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(double));
#endif
  write(&v, sizeof(double));
}

void Writer::writeString(const std::string& v) throw(IOException)
{
  writeInt((int)v.size());
  write(v.data(), v.size());
}

void Writer::writeObject(const Serializable& v) throw(IOException)
{
  return v.writeObject(*this);
}
