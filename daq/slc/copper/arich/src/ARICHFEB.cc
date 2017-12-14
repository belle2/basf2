#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <daq/slc/copper/HSLB.h>

#include "daq/slc/copper/arich/ARICHFEB.h"

using namespace Belle2;

void ARICHFEB::mysleep(int i)
{
  double d = i / 1000000.0f;
  //double d = i / 100000.0f;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double t = tv.tv_sec + tv.tv_usec / 1000000.0f;
  double  dt = 0;
  do {
    gettimeofday(&tv, NULL);
    double t1 = (tv.tv_sec + tv.tv_usec / 1000000.0f);
    dt = t1 - t;
  } while (d > dt);
}

ARICHFEB::ARICHFEB(HSLB& hslb, int num) : m_hslb(hslb), m_num(num) {}

int ARICHFEB::write(unsigned int address, unsigned char value)
{
  unsigned int b2adr = 0x300 + address;
  if (m_num >= 0) {
    unsigned int b2val = (m_num << 16) + 0x100 + value;
    //usleep(1);
    mysleep(100);
    m_hslb.writefee32(b2adr, b2val);
  }
  return 1;
}

unsigned int ARICHFEB::read(unsigned int address)
{
  unsigned int b2adr = 0x300 + address;
  if (m_num >= 0) {
    unsigned int b2val = (m_num << 16) + 0x00;
    //usleep(1);
    mysleep(100);
    m_hslb.writefee32(b2adr, b2val);
    //usleep(1);
    mysleep(100);
    //for (int i = 0; i < 10000; i++) {}
    return (m_hslb.readfee32(b2adr)) & 0xFF;
  }
  return -1;
}

int ARICHFEB::write1(unsigned int address, unsigned char value)
{
  return write(address, value);
}

int ARICHFEB::write2(unsigned int address, unsigned int value)
{
  int val = value & 0xff;
  write(address, val);
  val = (value >> 8) & 0xff;
  write(address + 1, val);
  return 0;
}

int ARICHFEB::write4(unsigned int address, unsigned int value)
{
  for (int i = 0; i < 4; i++) {
    int val = (value >> 8 * i) & 0xff;
    write(address + i, val);
  }
  return 0;
}

unsigned char ARICHFEB::read1(unsigned int address)
{
  return read(address);
}

unsigned int ARICHFEB::read2(unsigned int address)
{
  int val = read(address);
  val += read(address + 1) << 8;
  return val;
}

unsigned int ARICHFEB::read4(unsigned int address)
{
  int val = read(address);
  val += read(address + 1) << 8;
  val += read(address + 2) << 16;
  val += read(address + 3) << 24;
  return val;
}
