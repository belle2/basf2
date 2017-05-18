#include "daq/slc/copper/HSLB.h"
#include <mgt/hsreg.h>

#include <stdio.h>

#include "daq/slc/copper/top/B2L_access.h"

const int SCROD_UNDEFINED = 0xffffffff;
const int CARRIER_UNDEFINED = 0xffffffff;


int errorcountReadBack[HSLBMAX] = {0, 0, 0, 0};
int errorcountTimeOut[HSLBMAX] = {0, 0 , 0 , 0};
int errorcountDeadBeef[HSLBMAX] = {0, 0 , 0 , 0};;
int transactioncount[HSLBMAX] = {0, 0 , 0 , 0};;

unsigned sizeReadIgnoreRegs = sizeof(ReadIgnoreRegs) / sizeof(int);


int B2L_FullAddr(const int registerAddr, const int carrier, const int asic)
{
  return registerAddr + carrier * 0x2000 + asic * 0x400;
}

int B2L_Reset(const int scrod)
{
  int status = -1;
  Belle2::HSLB hslb;
  try {
    status = hslb.open(scrod);
    if (status > 0) {
      int csr = 0;
      hslb.reset_b2l(csr);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int B2L_ReadReg(const int registerAddr, const int scrod, const int carrier,
                const int asic, const unsigned read_retry)
{
  int status = -1;
  Belle2::HSLB hslb;
  try {
    status = hslb.open(scrod);
    if (status > 0) {
      status = B2L_ReadReg(hslb, registerAddr, carrier, asic, read_retry);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int B2L_ReadReg(Belle2::HSLB& hslb, const int registerAddr, const int carrier,
                const int asic, const unsigned read_retry)
{
  if (registerAddr == SCROD_UNDEFINED || registerAddr == CARRIER_UNDEFINED) {
    return -1;
  }

  int currentSCROD = hslb.get_finid();
  if (currentSCROD < HSLBMAX) transactioncount[currentSCROD]++;

  hslb.writefn32(HSREGL_RESET, 0x100);
  int addr = B2L_FullAddr(registerAddr, carrier, asic);
  int count = read_retry;
  unsigned readVal = hslb.readfee32(addr);
  while (readVal == TIMEOUT && count > 0) {
    readVal = hslb.readfee32(addr);
    count--;
  }
  if (count == 0 && currentSCROD < HSLBMAX) {
    errorcountTimeOut[currentSCROD]++;
    printf("ERROR: timeout occured when reading address %x", addr);
    return -2;
  }
  if (readVal == 0xDEADBEEF && currentSCROD < HSLBMAX) {
    errorcountDeadBeef[currentSCROD]++;
    printf("ERROR: deadbeef returned when reading address %x", addr);
    return -3;
  }
  return readVal;
}

int B2L_WriteReg(const int registerAddr, const int value, const int scrod, const int carrier,
                 const int asic, const unsigned write_retry)
{
  int status = -1;
  try {
    Belle2::HSLB hslb;
    int status = hslb.open(scrod);
    if (status > 0) {
      status = B2L_WriteReg(hslb, registerAddr, value, carrier, asic, write_retry);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int B2L_WriteReg(Belle2::HSLB& hslb, const int registerAddr, const int value, const int carrier,
                 const int asic, const unsigned write_retry)
{
  if (registerAddr == SCROD_UNDEFINED || registerAddr == CARRIER_UNDEFINED) {
    return -1;
  }

  int currentSCROD = hslb.get_finid();
  if (currentSCROD < HSLBMAX) transactioncount[currentSCROD]++;

  int addr = B2L_FullAddr(registerAddr, carrier, asic);
  hslb.writefn32(HSREGL_RESET, 0x100);
  hslb.writefee32(addr, value);
  //check value by reading it back
  unsigned ret = 0;
  for (unsigned i = 0; i < sizeReadIgnoreRegs; ++i) {
    if (addr == (int)ReadIgnoreRegs[i]) return 0;
  }
  for (unsigned i = 0; i < write_retry; ++i) {
    ret = B2L_ReadReg(hslb, registerAddr, carrier, asic);
    if ((int)ret == value) {
      return ret;
    } else {
      if (i == write_retry - 1 && currentSCROD < HSLBMAX) {
        errorcountReadBack[currentSCROD]++;
        printf("ERROR: readback error when writing register %x", addr);
        return -2;
      }
      hslb.writefn32(HSREGL_RESET, 0x100);
      hslb.writefee32(addr, value);
    }
  }
  return ret;
}

float TestPythonAdd(const int a, const int b)
{
  return a + b;
}
