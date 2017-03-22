/********************\
  ARICHFEBSA0x.cc

  update: 13/05/15

\********************/

#include <unistd.h>
#include <cstdio>

#include "daq/slc/copper/arich/ARICHFEBSA0x.h"
#include <daq/slc/copper/HSLB.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

ARICHFEBSA0x::ARICHFEBSA0x(HSLB& hslb, int febnum) : ARICHFEB(hslb, febnum) {}

void ARICHFEBSA0x::sel(unsigned char v, int usec)
{
  write1(ADDR_SEL, v);
  //usleep(usec); // necessary to wait selck signal to be sent
  ARICHFEB::mysleep(usec); // necessary to wait selck signal to be sent
}

void ARICHFEBSA0x::wparam(unsigned int data, int usec)
{
  write4(ADDR_WPARAM, data);
  //usleep(usec); // necessary to wait selck signal to be sent
  ARICHFEB::mysleep(usec); // necessary to wait selck signal to be sent
}

unsigned int ARICHFEBSA0x::rparam()
{
  return read4(ADDR_RPARAM);
}

void ARICHFEBSA0x::setcsr1(int value, unsigned int mask, unsigned int shift)
{
  unsigned char v = csr1();
  v = (v & (~mask)) | ((value << shift) & mask);
  csr1(v);
}

unsigned int ARICHFEBSA0x::select(unsigned int chip, int ch, int usec)
{
  int cid = (ch < 0) ? 0 : ch + 1;
  unsigned int val = ((chip & 0x3) << 6) | (cid & 0x3f);
  sel(val, usec);
  return val;
}

unsigned int ARICHFEBSA0x::setparam(unsigned int chip, int ch, unsigned int data)
{
  select(chip, ch);
  wparam(data);
  prmset();
  return rparam();
}

unsigned int ARICHFEBSA0x::ndro_param(unsigned int chip, int ch)
{
  select(chip, ch);
  ndro();
  return rparam();
}

unsigned int ARICHFEBSA0x::adc()
{
  write1(ADDR_ADC_DATA0, 0);
  return read2(ADDR_ADC_DATA0);
}

double ARICHFEBSA0x::voltage(unsigned int i_mux)
{
  mux(i_mux);
  unsigned int v = adc();
  return ADC_VCC * ((double)v + 0.5) / 4096;
}

void ARICHFEBSA0x::go()
{
  double mona = voltage(0x00);
  double monb = voltage(0x01);
  double mond = voltage(0x02);
  double monc = voltage(0x03);
  double vdd = voltage(0x10);
  double vp2 = voltage(0x11);
  double vm2 = 2 * voltage(0x12) - ADC_VCC;
  double vss = 2 * voltage(0x13) - ADC_VCC;
  double vth1 = voltage(0x20);
  double vth2 = voltage(0x30);
  double vcc12 = voltage(0x40);
  double vcc15 = voltage(0x50);
  double vcc25 = voltage(0x60);
  double vp38 = 2 * voltage(0x70);

  LogFile::info("slot %d", m_num);
  LogFile::info("monA %6.3f V    monB %6.3f V   monC %6.3f V   monD %6.3f",
                mona, monb, monc, mond);
  LogFile::info("VTH1 %6.3f V    VTH2 %6.3f V", vth1, vth2);
  LogFile::info("VDD  %6.3f V    V+2  %6.3f V   V-2  %6.3f V   VSS  %6.3f",
                vdd, vp2, vm2, vss);
  LogFile::info("V12  %6.3f V    V15  %6.3f V   V25  %6.3f V   V+38 %6.3f",
                vcc12, vcc15, vcc25, vp38);
}
