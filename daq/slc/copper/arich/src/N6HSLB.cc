#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#include <daq/slc/copper/HSLB.h>

#include "daq/slc/copper/arich/N6HSLB.h"

using namespace Belle2;

N6HSLB::N6HSLB(HSLB& hslb) : m_hslb(hslb) {}

int N6HSLB::write(unsigned int address, unsigned char value)
{
  unsigned int b2adr = 0x300 + address;
  unsigned int b2val = 0x30000100 + value;
  printf("write : 0x%x << 0x%x\n", b2adr, b2val);
  m_hslb.writefee32(b2adr, b2val);
  return 0;
}

unsigned int N6HSLB::read(unsigned int address)
{
  unsigned int b2adr = 0x300 + address;
  unsigned int b2val = 0x30000000;
  printf("write : 0x%x << 0x%x\n", b2adr, b2val);
  m_hslb.writefee32(b2adr, b2val);
  b2val = m_hslb.readfee32(b2adr);
  printf("read : 0x%x >> 0x%x\n", b2adr, b2val);
  return b2val;
}

int N6HSLB::write1(unsigned int address, unsigned char value)
{
  return write(address, value);
}

int N6HSLB::write2(unsigned int address, unsigned int value)
{
  int val = value & 0xff;
  write(address, val);
  val = (value >> 8) & 0xff;
  write(address + 1, val);
  return 0;
}

int N6HSLB::write4(unsigned int address, unsigned int value)
{
  for (int i = 0; i < 4; i++) {
    int val = (value >> 8 * i) & 0xff;
    write(address + i, val);
  }
  return 0;
}

unsigned char N6HSLB::read1(unsigned int address)
{
  return read(address);
}

unsigned int N6HSLB::read2(unsigned int address)
{
  int val = read(address);
  val += read(address + 1) << 8;
  return val;
}

unsigned int N6HSLB::read4(unsigned int address)
{
  int val = read(address);
  val += read(address + 1) << 8;
  val += read(address + 2) << 16;
  val += read(address + 3) << 24;
  return val;
}
