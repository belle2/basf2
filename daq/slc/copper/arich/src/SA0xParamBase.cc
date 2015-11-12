/********************\
  SA0xParamBase.cc

  update: 11/02/28

\********************/

#include <cstdio>

#include "daq/slc/copper/arich/SA0xParamBase.h"

void SA0xParamBase::setvalue(int value, unsigned int mask,
                             unsigned int shift)
{
  m_param = (m_param & (~mask)) | ((value << shift)&mask);
}

int SA0xParamBase::getrbvalue(unsigned int mask, unsigned int shift)
{
  return (m_rbparam & mask) >> shift;
}

void SA0xParamBase::dump(const char* name)
{
  printf("%s: set 0x%07x, readback 0x%07x\n", name, m_param, m_rbparam);
}

int SA0xParamBase::compare(bool verbose, const char* name)
{
  unsigned mask_variable = getMaskVariable();
  if ((m_param & mask_variable) != (m_rbparam & mask_variable)) {
    if (verbose) {
      printf("Error in %s: set 0x%07x, readback 0x%07x\n", name,
             m_param, m_rbparam);
    }
    return -1;
  }
  if (verbose) {
    printf("%s 0x%07x correctly loaded\n", name, m_param);
  }
  return 0;
}
