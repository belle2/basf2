/********************\
  N6SA0xHSLB.cc

  update: 13/05/15

\********************/

#include <unistd.h>

#include "daq/slc/copper/arich/N6SA0xHSLB.h"

using namespace Belle2;

N6SA0xHSLB::N6SA0xHSLB(HSLB& hslb) : N6HSLB(hslb) {}

void N6SA0xHSLB::sel(unsigned char v, int usec)
{
  write1(ADDR_SEL, v);
  usleep(usec); // necessary to wait selck signal to be sent
}

void N6SA0xHSLB::wparam(unsigned int data, int usec)
{
  write4(ADDR_WPARAM, data);
  usleep(usec); // necessary to wait selck signal to be sent
}

void N6SA0xHSLB::setcsr1(int value, unsigned int mask, unsigned int shift)
{
  unsigned char v = csr1();
  v = (v & (~mask)) | ((value << shift)&mask);
  csr1(v);
}

unsigned int N6SA0xHSLB::select(unsigned int chip, int ch, int usec)
{
  int cid = (ch < 0) ? 0 : ch + 1;
  unsigned int val = ((chip & 0x3) << 6) | (cid & 0x3f);
  sel(val, usec);
  return val;
}

unsigned int N6SA0xHSLB::setparam(unsigned int chip, int ch,
                                  unsigned int data)
{
  select(chip, ch);
  wparam(data);
  prmset();
  return rparam();
}

unsigned int N6SA0xHSLB::ndro_param(unsigned int chip, int ch)
{
  select(chip, ch);
  ndro();
  return rparam();
}
