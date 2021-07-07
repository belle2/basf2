/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/Reader.h"

#include "daq/slc/base/Serializable.h"

#include <unistd.h>

#define __ENDIAN_L__

using namespace Belle2;

void Reader::reverse(void* buf, size_t len)
{
  char tmp;
  for (int c = 0 ; c < (int)len / 2 ; c ++) {
    tmp = *((char*)buf + c);
    *((char*)buf + c) = *((char*)buf + len - 1 - c);
    *((char*)buf + len - 1 - c) = tmp;
  }
}

bool Reader::readBool()
{
  bool v;
  read(&v, sizeof(bool));
  return v;
}

char Reader::readChar()
{
  char v;
  read(&v, sizeof(char));
  return v;
}

unsigned char Reader::readUChar()
{
  unsigned char v;
  read(&v, sizeof(unsigned char));
  return v;
}

short Reader::readShort()
{
  short v;
  read(&v, sizeof(short));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(short));
#endif
  return v;
}

unsigned short Reader::readUShort()
{
  unsigned short v;
  read(&v, sizeof(unsigned short));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned short));
#endif
  return v;
}

int Reader::readInt()
{
  int v;
  read(&v, sizeof(int));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(int));
#endif
  return v;
}

unsigned int Reader::readUInt()
{
  unsigned int v;
  read(&v, sizeof(unsigned int));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned int));
#endif
  return v;
}

long long Reader::readLong()
{
  long long v;
  read(&v, sizeof(long long));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(long long));
#endif
  return v;
}

unsigned long long Reader::readULong()
{
  unsigned long long v;
  read(&v, sizeof(unsigned long long));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(unsigned long long));
#endif
  return v;
}

float Reader::readFloat()
{
  float v;
  read(&v, sizeof(float));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(float));
#endif
  return v;
}

double Reader::readDouble()
{
  double v;
  read(&v, sizeof(double));
#if defined(__ENDIAN_L__)
  reverse(&v, sizeof(double));
#endif
  return v;
}

const std::string Reader::readString()
{
  int count = readInt();
  char str[1025];
  std::string v;
  while (count > 0) {
    int c = (count > 1024) ? 1024 : count;
    read(str, c);
    v.append(str, 0, c);
    count -= c;
  }
  return v;
}

void Reader::readObject(Serializable& v)
{
  v.readObject(*this);
}
