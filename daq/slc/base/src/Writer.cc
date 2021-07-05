/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/Writer.h"

#include "daq/slc/base/Serializable.h"

#include <unistd.h>

#define __ENDIAN_L__

using namespace Belle2;

void Writer::reverse(void* buf, size_t len)
{
  char tmp;
  for (int c = 0 ; c < (int)len / 2 ; c ++) {
    tmp = *((char*)buf + c);
    *((char*)buf + c) = *((char*)buf + len - 1 - c);
    *((char*)buf + len - 1 - c) = tmp;
  }
}

void Writer::writeBool(bool v)
{
  write(&v, sizeof(bool));
}

void Writer::writeChar(char v)
{
  write(&v, sizeof(char));
}

void Writer::writeUChar(unsigned char v)
{
  write(&v, sizeof(unsigned char));
}

void Writer::writeShort(short v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(short));
#endif
  write(&v, sizeof(short));
}

void Writer::writeUShort(unsigned short v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned short));
#endif
  write(&v, sizeof(unsigned short));
}

void Writer::writeInt(int v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(int));
#endif
  write(&v, sizeof(int));
}

void Writer::writeUInt(unsigned int v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned int));
#endif
  write(&v, sizeof(unsigned int));
}

void Writer::writeLong(long long v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(long long));
#endif
  write(&v, sizeof(long long));
}

void Writer::writeULong(unsigned long long v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned long long));
#endif
  write(&v, sizeof(unsigned long long));
}

void Writer::writeFloat(float v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(float));
#endif
  write(&v, sizeof(float));
}

void Writer::writeDouble(double v)
{
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(double));
#endif
  write(&v, sizeof(double));
}

void Writer::writeString(const std::string& v)
{
  writeInt((int)v.size());
  write(v.data(), v.size());
}

void Writer::writeObject(const Serializable& v)
{
  return v.writeObject(*this);
}
